/*

SleepLib Preferences Header

Author: Mark Watkins <jedimark64@users.sourceforge.net>
License: GPL

*/

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QString>
#include <QVariant>
#include <QHash>
#include <QDomElement>
#include <QDomDocument>
#include <map>


const QString AppName="SleepyHead"; // Outer tag of XML files
const QString AppRoot="SleepApp";    // The Folder Name

extern const QString & GetAppRoot(); //returns app root path plus trailing path separator.

inline QString PrefMacro(QString s)
{
    return "{"+s+"}";
}

//! \brief Returns a QString containing the Username, according to the Operating System
const QString & getUserName();


/*! \class Preferences
    \author Mark Watkins <jedimark_at_users.sourceforge.net>
    \brief Holds a group of preference variables
    */
class Preferences
{
public:
    //! \brief Constructs a Preferences object 'name', and remembers sets the filename
    Preferences(QString name,QString filename="");
    Preferences();
    virtual ~Preferences();

    //! \brief Returns a QString containing preference 'name', processing any {} macros
    const QString Get(QString name);

    //! \brief Returns the QVariant value of the selected preference.. Note, preference must exist, and will not expand {} macros
    QVariant & operator[](QString name) {
        return p_preferences[name];
    }

    //! \brief Sets the Preference 'name' to QVariant 'value'
    void Set(QString name,QVariant value) {
        p_preferences[name]=value;
    }

    //! \brief Returns true if preference 'name' exists
    bool Exists(QString name) {
        return (p_preferences.contains(name));
    }

    //! \brief Returns true if preference 'name' exists, and contains a boolean true value
    bool ExistsAndTrue(QString name) {
        QHash<QString,QVariant>::iterator i=p_preferences.find(name);
        if (i==p_preferences.end()) return false;
        return i.value().toBool();
    }

    //! \brief Removes preference 'name' from this Preferences group
    void Erase(QString name) {
        QHash<QString,QVariant>::iterator i=p_preferences.find(name);
        if (i!=p_preferences.end())
            p_preferences.erase(i);
    }

    //! \brief Derive from this to handle Loading of any custom XML sections
    virtual void ExtraLoad(QDomElement & root) { root=root; }

    //! \brief Derive from this to handle Saving of any custom XML sections
    //! \return Must return a QDomElement to be inserted into the generated XML
    virtual QDomElement ExtraSave(QDomDocument & doc) { doc=doc; QDomElement e; return e; }

    //! \brief Opens, processes the XML for this Preferences group, loading all preferences stored therein.
    //! \note If filename is empty, it will use the one specified in the constructor
    //! \returns true if succesful
    virtual bool Open(QString filename="");

    //! \brief Saves all preferences to XML file.
    //! \note If filename is empty, it will use the one specified in the constructor
    //! \returns true if succesful
    virtual bool Save(QString filename="");

    //! \note Sets a comment string whici will be stored in the XML
    void SetComment(const QString & str) {
        p_comment=str;
    }

    //! \brief Finds a given preference.
    //! \returns a QHash<QString,QString>::iterator pointing to the preference named 'key', or an empty end() iterator
    inline QHash<QString,QVariant>::iterator find(QString key) { return p_preferences.find(key); }

    //! \brief Returns an empty iterator pointing to the end of the preferences list
    inline QHash<QString,QVariant>::iterator end() { return p_preferences.end(); }

    //! \brief Returns an iterator pointing to the first item in the preferences list
    inline QHash<QString,QVariant>::iterator begin() { return p_preferences.begin(); }

    //int  GetCode(QString name); // For registering/looking up new preference code.

    //! \brief Stores all the variants indexed by a QString name for this Preferences object
    QHash<QString,QVariant> p_preferences;

protected:
    //QHash<int,QString> p_codes;
    QString p_comment;
    QString p_name;
    QString p_filename;
    QString p_path;
};

enum PrefType { PT_Checkbox, PT_Integer, PT_Number, PT_Date, PT_Time, PT_DateTime, PT_LineEdit, PT_TextEdit, PT_Dropdown };

/*! \class Preference
    \brief Holds a single preference
    \note This is a work in progress to clean up preferences system
    */
class Preference
{
public:
    Preference() {
        m_pref=NULL;
    }
    Preference(const Preference & copy) {
        m_pref=copy.m_pref;
        m_code=copy.m_code;
        m_type=copy.m_type;
        m_label=copy.m_label;
        m_tooltip=copy.m_tooltip;
        m_defaultValue=copy.m_defaultValue;
    }
    Preference(Preferences * pref, QString code, PrefType type, QString label, QString tooltip, QVariant default_value);
    ~Preference() {}

    QString code() { return m_code; }

    void setValue(QVariant v);
    QVariant & value();

    PrefType type() { return m_type; }
    QString label() { return m_label; }
    QString tooltip() { return m_tooltip; }
    QVariant defaultValue() { return m_defaultValue; }
protected:
    Preferences * m_pref;
    QString m_code;
    PrefType m_type;
    QString m_label;
    QString m_tooltip;
    QVariant m_defaultValue;
};

Q_DECLARE_METATYPE(Preference)

//! \brief Main Preferences Object used throughout the application
extern Preferences PREF;

//! \brief Layout Preferences Object used throughout the application
extern Preferences LAYOUT;

#endif // PREFERENCES_H

