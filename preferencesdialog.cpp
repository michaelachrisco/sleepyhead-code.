#include <QLabel>
#include <QColorDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QProcess>
#include <QDesktopServices>
#include <QFileDialog>
#include <QTextStream>
#include "preferencesdialog.h"


#include <Graphs/gGraphView.h>
#include <mainwindow.h>
#include "ui_preferencesdialog.h"
#include "SleepLib/machine_common.h"

extern QFont * defaultfont;
extern QFont * mediumfont;
extern QFont * bigfont;
extern MainWindow * mainwin;

PreferencesDialog::PreferencesDialog(QWidget *parent,Profile * _profile) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    profile(_profile)
{
    ui->setupUi(this);

    {
        QString filename=PROFILE.Get("{DataFolder}/ImportLocations.txt");
        QFile file(filename);
        file.open(QFile::ReadOnly);
        QTextStream textStream(&file);
        while (1) {
            QString line = textStream.readLine();
             if (line.isNull())
                 break;
             else if (line.isEmpty())
                 continue;
             else {
                 importLocations.append(line);
             }
        };
        file.close();
    }
    importModel=new QStringListModel(importLocations,this);
    ui->importListWidget->setModel(importModel);
    //ui->tabWidget->removeTab(3);

    Q_ASSERT(profile!=NULL);
    ui->tabWidget->setCurrentIndex(0);
    int i=ui->unitCombo->findText((*profile)["UnitSystem"].toString());
    if (i<0) i=0;
    ui->unitCombo->setCurrentIndex(i);

    //i=ui->timeZoneCombo->findText((*profile)["TimeZone"].toString());
    //ui->timeZoneCombo->setCurrentIndex(i);

    bool  ok;
    double v;
    v=(*profile)["SPO2DropPercentage"].toDouble(&ok);
    if (!ok) v=4;
    ui->spo2Drop->setValue(v);
    v=(*profile)["SPO2DropDuration"].toDouble(&ok);
    if (!ok) v=5;
    ui->spo2DropTime->setValue(v);
    v=(*profile)["PulseChangeBPM"].toDouble(&ok);
    if (!ok) v=5;
    ui->pulseChange->setValue(v);
    v=(*profile)["PulseChangeDuration"].toDouble(&ok);
    if (!ok) v=5;
    ui->pulseChangeTime->setValue(v);

    QTime t=(*profile)["DaySplitTime"].toTime();
    ui->timeEdit->setTime(t);
    int val;

    val=(*profile)["CombineCloserSessions"].toInt();
    ui->combineSlider->setValue(val);
    if (val>0) {
        ui->combineLCD->display(val);
    } else ui->combineLCD->display(tr("OFF"));


    val=(*profile)["IgnoreShorterSessions"].toInt();
    ui->IgnoreSlider->setValue(val);

    ui->applicationFont->setCurrentFont(QApplication::font());
    ui->applicationFont->setFont(QApplication::font());
    ui->applicationFontSize->setValue(QApplication::font().pointSize());
    ui->applicationFontBold->setChecked(QApplication::font().weight()==QFont::Bold);
    ui->applicationFontItalic->setChecked(QApplication::font().italic());

    ui->graphFont->setCurrentFont(*defaultfont);
    ui->graphFont->setFont(*defaultfont);
    ui->graphFontSize->setValue(defaultfont->pointSize());
    ui->graphFontBold->setChecked(defaultfont->weight()==QFont::Bold);
    ui->graphFontItalic->setChecked(defaultfont->italic());

    ui->titleFont->setCurrentFont(*mediumfont);
    ui->titleFont->setFont(*mediumfont);
    ui->titleFontSize->setValue(mediumfont->pointSize());
    ui->titleFontBold->setChecked(mediumfont->weight()==QFont::Bold);
    ui->titleFontItalic->setChecked(mediumfont->italic());

    ui->bigFont->setCurrentFont(*bigfont);
    ui->bigFont->setFont(*bigfont);
    ui->bigFontSize->setValue(bigfont->pointSize());
    ui->bigFontBold->setChecked(bigfont->weight()==QFont::Bold);
    ui->bigFontItalic->setChecked(bigfont->italic());

    if (!(*profile).Exists("SkipEmptyDays")) (*profile)["SkipEmptyDays"]=true;
    ui->skipEmptyDays->setChecked((*profile)["SkipEmptyDays"].toBool());

    if (!(*profile).Exists("SquareWavePlots")) (*profile)["SquareWavePlots"]=true;
    ui->squareWavePlots->setChecked((*profile)["SquareWavePlots"].toBool());

    if (!PREF.Exists("Updates_AutoCheck")) PREF["Updates_AutoCheck"]=true;
    ui->automaticallyCheckUpdates->setChecked(PREF["Updates_AutoCheck"].toBool());

    if (!PREF.Exists("SkipLoginScreen")) PREF["SkipLoginScreen"]=false;
    ui->skipLoginScreen->setChecked(PREF["SkipLoginScreen"].toBool());

    if (!PREF.Exists("Updates_CheckFrequency")) PREF["Updates_CheckFrequency"]=3;
    ui->updateCheckEvery->setValue(PREF["Updates_CheckFrequency"].toInt());
    if (PREF.Exists("Updates_LastChecked")) {
        RefreshLastChecked();
    } else ui->updateLastChecked->setText("Never");

    if (val>0) {
        ui->IgnoreLCD->display(val);
    } else ui->IgnoreLCD->display(tr("OFF"));

    ui->overlayFlagsCombo->setCurrentIndex((*profile)["AlwaysShowOverlayBars"].toInt());
    ui->useAntiAliasing->setChecked((*profile)["UseAntiAliasing"].toBool());
    ui->memoryHogCheckbox->setChecked((*profile)["MemoryHog"].toBool());
    ui->useGraphSnapshots->setChecked((*profile)["EnableGraphSnapshots"].toBool());
    ui->intentionalLeakEdit->setValue((*profile)["IntentionalLeak"].toDouble());
    ui->useMultithreading->setChecked((*profile)["EnableMultithreading"].toBool());

    ui->oximetryGroupBox->setChecked((*profile)["EnableOximetry"].toBool());
    ui->oximetrySync->setChecked((*profile)["SyncOximetry"].toBool());
    ui->oximetryType->setCurrentIndex(ui->oximetryType->findText((*profile)["OximeterType"].toString(),Qt::MatchExactly));

    ui->eventTable->setColumnWidth(0,40);
    ui->eventTable->setColumnWidth(1,55);
    ui->eventTable->setColumnHidden(3,true);
    int row=0;
    QTableWidgetItem *item;
    QHash<QString, schema::Channel *>::iterator ci;
    for (ci=schema::channel.names.begin();ci!=schema::channel.names.end();ci++) {
        if (ci.value()->type()==schema::DATA) {
            ui->eventTable->insertRow(row);
            int id=ci.value()->id();
            ui->eventTable->setItem(row,3,new QTableWidgetItem(QString::number(id)));
            item=new QTableWidgetItem(ci.value()->description());
            ui->eventTable->setItem(row,2,item);
            QCheckBox *c=new QCheckBox(ui->eventTable);
            c->setChecked(true);
            QLabel *pb=new QLabel(ui->eventTable);
            pb->setText("foo");
            ui->eventTable->setCellWidget(row,0,c);
            ui->eventTable->setCellWidget(row,1,pb);


            QColor a=ci.value()->defaultColor();//(rand() % 255, rand() % 255, rand() % 255, 255);
            QPalette p(a,a,a,a,a,a,a);

            pb->setPalette(p);
            pb->setAutoFillBackground(true);
            pb->setBackgroundRole(QPalette::Background);
            row++;
        }
    }
/*    QLocale locale=QLocale::system();
    QString shortformat=locale.dateFormat(QLocale::ShortFormat);
    if (!shortformat.toLower().contains("yyyy")) {
        shortformat.replace("yy","yyyy");
    }*/

    graphFilterModel=new MySortFilterProxyModel(this);
    graphModel=new QStandardItemModel(this);
    graphFilterModel->setSourceModel(graphModel);
    graphFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    graphFilterModel->setFilterKeyColumn(0);
    ui->graphView->setModel(graphFilterModel);



    resetGraphModel();
//    tree->sortByColumn(0,Qt::AscendingOrder);

}


PreferencesDialog::~PreferencesDialog()
{
    disconnect(graphModel,SIGNAL(itemChanged(QStandardItem*)),this,SLOT(on_graphModel_changed(QStandardItem*)));
    delete ui;
}

void PreferencesDialog::on_eventTable_doubleClicked(const QModelIndex &index)
{
    int row=index.row();
    int col=index.column();
    bool ok;
    int id=ui->eventTable->item(row,3)->text().toInt(&ok);
    if (col==1) {
        QWidget *w=ui->eventTable->cellWidget(row,col);
        QColorDialog a;
        QColor color=w->palette().background().color();
        a.setCurrentColor(color);
        if (a.exec()==QColorDialog::Accepted) {
            QColor c=a.currentColor();
            QPalette p(c,c,c,c,c,c,c);
            w->setPalette(p);
            m_new_colors[id]=c;
            //qDebug() << "Color accepted" << col << id;
        }
    }
}

void PreferencesDialog::Save()
{
    bool needs_restart=false;

    (*profile)["UnitSystem"]=ui->unitCombo->currentText();
    //(*profile)["TimeZone"]=ui->timeZoneCombo->currentText();

    if ((*profile)["CombineCloserSessions"].toInt()!=ui->combineSlider->value()) needs_restart=true;
    if ((*profile)["IgnoreShorterSessions"].toInt()!=ui->IgnoreSlider->value()) needs_restart=true;

    (*profile)["CombineCloserSessions"]=ui->combineSlider->value();
    (*profile)["IgnoreShorterSessions"]=ui->IgnoreSlider->value();

    (*profile)["MemoryHog"]=ui->memoryHogCheckbox->isChecked();

    if ((*profile)["DaySplitTime"].toTime()!=ui->timeEdit->time()) needs_restart=true;

    (*profile)["DaySplitTime"]=ui->timeEdit->time();

    (*profile)["AlwaysShowOverlayBars"]=ui->overlayFlagsCombo->currentIndex();
    (*profile)["UseAntiAliasing"]=ui->useAntiAliasing->isChecked();
    (*profile)["MemoryHog"]=ui->memoryHogCheckbox->isChecked();
    (*profile)["EnableGraphSnapshots"]=ui->useGraphSnapshots->isChecked();
    (*profile)["IntentionalLeak"]=ui->intentionalLeakEdit->value();
    (*profile)["EnableMultithreading"]=ui->useMultithreading->isChecked();
    (*profile)["EnableOximetry"]=ui->oximetryGroupBox->isChecked();
    (*profile)["SyncOximetry"]=ui->oximetrySync->isChecked();
    (*profile)["OximeterType"]=ui->oximetryType->currentText();

    (*profile)["SPO2DropPercentage"]=ui->spo2Drop->value();
    (*profile)["SPO2DropDuration"]=ui->spo2DropTime->value();
    (*profile)["PulseChangeBPM"]=ui->pulseChange->value();
    (*profile)["PulseChangeDuration"]=ui->pulseChangeTime->value();

    PREF["SkipLoginScreen"]=ui->skipLoginScreen->isChecked();

    if (ui->squareWavePlots->isChecked() != (*profile)["SquareWavePlots"].toBool()) {
        needs_restart=true;
    }
    (*profile)["SquareWavePlots"]=ui->squareWavePlots->isChecked();

    (*profile)["SkipEmptyDays"]=ui->skipEmptyDays->isChecked();
    PREF["Updates_AutoCheck"]=ui->automaticallyCheckUpdates->isChecked();
    PREF["Updates_CheckFrequency"]=ui->updateCheckEvery->value();

    PREF["Fonts_Application_Name"]=ui->applicationFont->currentText();
    PREF["Fonts_Application_Size"]=ui->applicationFontSize->value();
    PREF["Fonts_Application_Bold"]=ui->applicationFontBold->isChecked();
    PREF["Fonts_Application_Italic"]=ui->applicationFontItalic->isChecked();

    PREF["Fonts_Graph_Name"]=ui->graphFont->currentText();
    PREF["Fonts_Graph_Size"]=ui->graphFontSize->value();
    PREF["Fonts_Graph_Bold"]=ui->graphFontBold->isChecked();
    PREF["Fonts_Graph_Italic"]=ui->graphFontItalic->isChecked();

    PREF["Fonts_Title_Name"]=ui->titleFont->currentText();
    PREF["Fonts_Title_Size"]=ui->titleFontSize->value();
    PREF["Fonts_Title_Bold"]=ui->titleFontBold->isChecked();
    PREF["Fonts_Title_Italic"]=ui->titleFontItalic->isChecked();

    PREF["Fonts_Big_Name"]=ui->bigFont->currentText();
    PREF["Fonts_Big_Size"]=ui->bigFontSize->value();
    PREF["Fonts_Big_Bold"]=ui->bigFontBold->isChecked();
    PREF["Fonts_Big_Italic"]=ui->bigFontItalic->isChecked();

    QFont font=ui->applicationFont->currentFont();
    font.setPointSize(ui->applicationFontSize->value());
    font.setWeight(ui->applicationFontBold->isChecked()?QFont::Bold : QFont::Normal);
    font.setItalic(ui->applicationFontItalic->isChecked());
    QApplication::setFont(font);

    *defaultfont=ui->graphFont->currentFont();
    defaultfont->setPointSize(ui->graphFontSize->value());
    defaultfont->setWeight(ui->graphFontBold->isChecked()?QFont::Bold : QFont::Normal);
    defaultfont->setItalic(ui->graphFontItalic->isChecked());

    *mediumfont=ui->titleFont->currentFont();
    mediumfont->setPointSize(ui->titleFontSize->value());
    mediumfont->setWeight(ui->titleFontBold->isChecked()?QFont::Bold : QFont::Normal);
    mediumfont->setItalic(ui->titleFontItalic->isChecked());

    *bigfont=ui->bigFont->currentFont();
    bigfont->setPointSize(ui->bigFontSize->value());
    bigfont->setWeight(ui->bigFontBold->isChecked()?QFont::Bold : QFont::Normal);
    bigfont->setItalic(ui->bigFontItalic->isChecked());

    // Process color changes
    for (QHash<int,QColor>::iterator i=m_new_colors.begin();i!=m_new_colors.end();i++) {
        schema::Channel &chan=schema::channel[i.key()];
        if (!chan.isNull()) {
            qDebug() << "TODO: Change" << chan.name() << "color to" << i.value();
            chan.setDefaultColor(i.value());
        }
    }

    //qDebug() << "TODO: Save channels.xml to update channel data";

    {
        QString filename=PROFILE.Get("{DataFolder}/ImportLocations.txt");
        QFile file(filename);
        file.open(QFile::WriteOnly);
        QTextStream ts(&file);
        for (int i=0;i<importLocations.size();i++) {
            ts << importLocations[i] << endl;
            //file.write(importLocations[i].toUtf8());
        }
        file.close();
    }

    profile->Save();
    PREF.Save();

    if (needs_restart) {
        if (QMessageBox::question(this,"Restart Required","One or more of the changes you have made will require this application to be restarted, in order for these changes to come into effect.\nWould you like do this now?",QMessageBox::Yes,QMessageBox::No)==QMessageBox::Yes) {
            QString apppath;
    #ifdef Q_OS_MAC
            // In Mac OS the full path of aplication binary is:
            //    <base-path>/myApp.app/Contents/MacOS/myApp

            // prune the extra bits to just get the app bundle path
            apppath=QApplication::instance()->applicationDirPath().section("/",0,-3);

            QStringList args;
            args << "-n" << apppath; // -n option is important, as it opens a new process

            if (QProcess::startDetached("/usr/bin/open",args)) {
                QApplication::instance()->exit();
            } else QMessageBox::warning(this,"Gah!","If you can read this, the restart command didn't work. Your going to have to do it yourself manually.",QMessageBox::Ok);

    #else
            apppath=QApplication::instance()->applicationFilePath();

            // If this doesn't work on windoze, try uncommenting this method
            // Technically should be the same thing..

            //if (QDesktopServices::openUrl(apppath)) {
            //    QApplication::instance()->exit();
            //} else
            if (QProcess::startDetached(apppath)) {
                QApplication::instance()->exit();
            } else QMessageBox::warning(this,"Gah!","If you can read this, the restart command didn't work. Your going to have to do it yourself manually.",QMessageBox::Ok);
    #endif
        }
    }
}

void PreferencesDialog::on_combineSlider_valueChanged(int position)
{
    if (position>0) {
        ui->combineLCD->display(position);
    } else ui->combineLCD->display(tr("OFF"));
}

void PreferencesDialog::on_IgnoreSlider_valueChanged(int position)
{
    if (position>0) {
        ui->IgnoreLCD->display(position);
    } else ui->IgnoreLCD->display(tr("OFF"));
}

void PreferencesDialog::on_useGraphSnapshots_toggled(bool checked)
{
    if (checked
        && !(*profile)["EnableGraphSnapshots"].toBool()
        && QMessageBox::question(this,"Warning","The Graph Snapshots feature (used by the Pie Chart in Daily stats panel) has been known to not work on some older computers.\n\nIf you experience a crash because of it, you will have to remove your SleepApp folder and recreate your profile.\n\n(I'm fairly sure this Qt bug is fixed now, but this has not been tested enough. If you have previously seen the pie chart, it's perfectly ok.)\n\nAre you sure you want to enable it?",QMessageBox::Yes,QMessageBox::No)==QMessageBox::No) {
        ui->useGraphSnapshots->setChecked(false);
    }
}

#include "mainwindow.h"
extern MainWindow * mainwin;
void PreferencesDialog::RefreshLastChecked()
{
    ui->updateLastChecked->setText(PREF["Updates_LastChecked"].toDateTime().toString(Qt::SystemLocaleLongDate));
}

void PreferencesDialog::on_checkForUpdatesButton_clicked()
{
    mainwin->statusBar()->showMessage("Checking for Updates");
    ui->updateLastChecked->setText("Checking for Updates");
    mainwin->CheckForUpdates();
}

void PreferencesDialog::on_addImportLocation_clicked()
{
    QString dir=QFileDialog::getExistingDirectory(this,"Add this Location to the Import List","",QFileDialog::ShowDirsOnly);

    if (!dir.isEmpty()) {
        if (!importLocations.contains(dir)) {
            importLocations.append(dir);
            importModel->setStringList(importLocations);
        }
    }
}

void PreferencesDialog::on_removeImportLocation_clicked()
{
    if (ui->importListWidget->currentIndex().isValid()) {
        QString dir=ui->importListWidget->currentIndex().data().toString();
        importModel->removeRow(ui->importListWidget->currentIndex().row());
        importLocations.removeAll(dir);
    }
}


void PreferencesDialog::on_graphView_activated(const QModelIndex &index)
{
    QString a=index.data().toString();
    qDebug() << "Could do something here with" << a;
}

void PreferencesDialog::on_graphFilter_textChanged(const QString &arg1)
{
    graphFilterModel->setFilterFixedString(arg1);
}


MySortFilterProxyModel::MySortFilterProxyModel(QObject *parent)
    :QSortFilterProxyModel(parent)
{

}

bool MySortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (source_parent == qobject_cast<QStandardItemModel*>(sourceModel())->invisibleRootItem()->index()) {
         // always accept children of rootitem, since we want to filter their children
         return true;
    }

    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

void PreferencesDialog::on_graphModel_changed(QStandardItem * item)
{
    QModelIndex index=item->index();

    gGraphView *gv=NULL;
    bool ok;

    const QModelIndex & row=index.sibling(index.row(),0);
    bool checked=row.data(Qt::CheckStateRole)!=0;
    QString name=row.data().toString();

    int group=row.data(Qt::UserRole+1).toInt();
    int id=row.data(Qt::UserRole+2).toInt();
    switch(group) {
        case 0: gv=mainwin->getDaily()->graphView(); break;
        case 1: gv=mainwin->getOverview()->graphView(); break;
        case 2: gv=mainwin->getOximetry()->graphView(); break;
    default: ;
    }

    if (!gv)
        return;

    gGraph *graph=(*gv)[id];
    if (!graph)
        return;

    if (graph->visible()!=checked) {
        graph->setVisible(checked);
    }

    EventDataType val;

    if (index.column()==1) {
        val=index.data().toDouble(&ok);

        if (!ok) {
            graphModel->setData(index,QString::number(graph->rec_miny,'f',1));
            ui->graphView->update();
        }  else {
            if ((val < graph->rec_maxy) || (val==0)) {
                graph->setRecMinY(val);
            } else {
                graphModel->setData(index,QString::number(graph->rec_miny,'f',1));
                ui->graphView->update();
            }
        }
    } else if (index.column()==2) {
        val=index.data().toDouble(&ok);
        if (!ok) {
            graphModel->setData(index,QString::number(graph->rec_maxy,'f',1));
            ui->graphView->update();
        }  else {
            if ((val > graph->rec_miny) || (val==0)) {
                graph->setRecMaxY(val);
            } else {
                graphModel->setData(index,QString::number(graph->rec_maxy,'f',1));
                ui->graphView->update();
            }
        }

    }
    gv->updateScale();
//    qDebug() << name << checked;
}

void PreferencesDialog::resetGraphModel()
{

    graphModel->clear();
    QStandardItem *daily=new QStandardItem("Daily Graphs");
    QStandardItem *overview=new QStandardItem("Overview Graphs");
    daily->setEditable(false);
    overview->setEditable(false);

    graphModel->appendRow(daily);
    graphModel->appendRow(overview);
    connect(graphModel,SIGNAL(itemChanged(QStandardItem*)),this,SLOT(on_graphModel_changed(QStandardItem*)));

    ui->graphView->setAlternatingRowColors(true);

    ui->graphView->setFirstColumnSpanned(0,daily->index(),true);
    graphModel->setColumnCount(3);
    QStringList headers;
    headers.append("Graph");
    headers.append("Min");
    headers.append("Max");
    graphModel->setHorizontalHeaderLabels(headers);
    ui->graphView->setColumnWidth(0,250);
    ui->graphView->setColumnWidth(1,50);
    ui->graphView->setColumnWidth(2,50);

    gGraphView *gv=mainwin->getDaily()->graphView();
    for (int i=0;i<gv->size();i++) {
        QList<QStandardItem *> items;
        QString title=(*gv)[i]->title();
        QStandardItem *it=new QStandardItem(title);
        it->setCheckable(true);
        it->setCheckState((*gv)[i]->visible() ? Qt::Checked : Qt::Unchecked);
        it->setEditable(false);
        it->setData(0,Qt::UserRole+1);
        it->setData(i,Qt::UserRole+2);
        items.push_back(it);

        if (title!="Event Flags") {

            it=new QStandardItem(QString::number((*gv)[i]->rec_miny,'f',1));
            it->setEditable(true);
            items.push_back(it);

            it=new QStandardItem(QString::number((*gv)[i]->rec_maxy,'f',1));
            it->setEditable(true);
            items.push_back(it);
        } else {
            it=new QStandardItem(tr("N/A")); // not applicable.
            it->setEditable(false);
            items.push_back(it);
            items.push_back(it->clone());
        }

        daily->insertRow(i,items);
    }

    gv=mainwin->getOverview()->graphView();
    for (int i=0;i<gv->size();i++) {
        QList<QStandardItem *> items;
        QStandardItem *it=new QStandardItem((*gv)[i]->title());
        it->setCheckable(true);
        it->setCheckState((*gv)[i]->visible() ? Qt::Checked : Qt::Unchecked);
        it->setEditable(false);
        items.push_back(it);
        it->setData(1,Qt::UserRole+1);
        it->setData(i,Qt::UserRole+2);

        it=new QStandardItem(QString::number((*gv)[i]->rec_miny,'f',1));
        it->setEditable(true);
        items.push_back(it);

        it=new QStandardItem(QString::number((*gv)[i]->rec_maxy,'f',1));
        it->setEditable(true);
        items.push_back(it);

        overview->insertRow(i,items);
    }
    if (mainwin->getOximetry()) {
        QStandardItem *oximetry=new QStandardItem("Oximetry Graphs");
        graphModel->appendRow(oximetry);
        oximetry->setEditable(false);
        gv=mainwin->getOximetry()->graphView();
        for (int i=0;i<gv->size();i++) {
            QList<QStandardItem *> items;
            QStandardItem *it=new QStandardItem((*gv)[i]->title());
            it->setCheckable(true);
            it->setCheckState((*gv)[i]->visible() ? Qt::Checked : Qt::Unchecked);
            it->setEditable(false);
            it->setData(2,Qt::UserRole+1);
            it->setData(i,Qt::UserRole+2);
            items.push_back(it);

            it=new QStandardItem(QString::number((*gv)[i]->rec_miny,'f',1));
            it->setEditable(true);
            items.push_back(it);

            it=new QStandardItem(QString::number((*gv)[i]->rec_maxy,'f',1));
            it->setEditable(true);
            items.push_back(it);

            oximetry->insertRow(i,items);
        }
    }

    ui->graphView->expandAll();
}

void PreferencesDialog::on_resetGraphButton_clicked()
{
    if (QMessageBox::question(this,"Confirmation","Are you sure you want to reset your graph preferences to the defaults?",QMessageBox::Yes,QMessageBox::No)==QMessageBox::Yes) {
        gGraphView *gv[3];
        gv[0]=mainwin->getDaily()->graphView();
        gv[1]=mainwin->getOverview()->graphView();
        gv[2]=mainwin->getOximetry()->graphView();
        for (int j=0;j<3;j++) {
            if (gv[j]!=NULL) {
                for (int i=0;i<gv[j]->size();i++) {
                    gGraph *g=(*(gv[j]))[i];
                    g->setRecMaxY(0);
                    g->setRecMinY(0);
                    g->setVisible(true);
                }
                gv[j]->updateScale();
            }
        }
        resetGraphModel();
        ui->graphView->update();
    }
 }
