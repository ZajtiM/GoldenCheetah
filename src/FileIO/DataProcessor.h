/*
 * Copyright (c) 2010 Mark Liversedge (liversedge@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _DataProcessor_h
#define _DataProcessor_h
#include "GoldenCheetah.h"

#include "RideFile.h"
#include "RideFileCommand.h"
#include "RideItem.h"

#include <QtGui>
#include <QWidget>
#include <QDialog>
#include <QDate>
#include <QDir>
#include <QLabel>
#include <QFile>
#include <QList>
#include <QTextEdit>
#include <QLineEdit>
#include <QMap>
#include <QVector>

// This file defines four classes:
//
// DataProcessorConfig is a base QWidget that must be supplied by the
// DataProcessor to enable the user to configure its options
//
// DataProcessor is an abstract base class for function-objects that take a
// rideFile and manipulate it. Examples include fixing gaps in recording or
// creating the .notes or .cpi file
//
// DataProcessorFactory is a singleton that maintains a mapping of
// all DataProcessor objects that can be applied to rideFiles
//
// ManualDataProcessorDialog is a dialog box to manually execute a
// dataprocessor on the current ride and is called from the mainWindow menus
// when no ride but DataProcessorConfig is provided it allows edit and confirm
// to apply to multiple ride in BatchProcessingDialog
//

// every data processor must supply a configuration Widget
// when its processorConfig member is called
class DataProcessorConfig : public QWidget
{
    Q_OBJECT

    public:
        DataProcessorConfig(QWidget *parent=0) : QWidget(parent) {}
        virtual ~DataProcessorConfig() {}
        virtual void readConfig() = 0;
        virtual void saveConfig() = 0;
};

// the data processor abstract base class
class DataProcessor
{
    public:
        enum Automation {
            Manual = 0,
            Auto = 1,
            Save = 2
        };

        DataProcessor() {}
        virtual ~DataProcessor() {}
        virtual bool postProcess(RideFile *, DataProcessorConfig*settings=0, QString op="") = 0;
        virtual DataProcessorConfig *processorConfig(QWidget *parent, const RideFile* ride = NULL) const = 0;
        virtual QString name() const = 0; // Localized Name for user interface
        virtual QString id() const = 0; // Unique Id for internal handling
        virtual QString legacyId() const { return QString(); }
        virtual bool isCoreProcessor() const { return true; }
        Automation getAutomation() const;
        void setAutomation(Automation automation);
        bool isAutomatedOnly() const;
        void setAutomatedOnly(bool automatedOnly);
        virtual QString explain() const = 0;

        static QString configKeyAutomatedOnly(const QString &id);
        static QString configKeyAutomation(const QString &id);
        static QString configKeyApply(const QString &id); // Legacy key, only required for migration. Use configKeyAutomation instead
};

// all data processors
class DataProcessorFactory {

    private:
        static DataProcessorFactory *instance_;
        static bool autoprocess;
        QMap<QString,DataProcessor*> processors;
        DataProcessorFactory() {}

    public:
        ~DataProcessorFactory();
        static DataProcessorFactory &instance();
        bool registerProcessor(DataProcessor *processor);
        void unregisterProcessor(QString name);
        DataProcessor* getProcessor(const QString &id) const;
        QMap<QString,DataProcessor*> getProcessors(bool coreProcessorsOnly = false) const;
        QList<DataProcessor*> getProcessorsSorted(bool coreProcessorsOnly = false) const;
        bool autoProcess(RideFile *, QString mode, QString op); // run auto processes (after open rideFile)
        void setAutoProcessRule(bool b) { autoprocess = b; } // allows to switch autoprocess off (e.g. for Upgrades)
};

class Context;
class ManualDataProcessorDialog : public QDialog
{
    Q_OBJECT
    G_OBJECT


    public:
        ManualDataProcessorDialog(Context *, QString, RideItem *, DataProcessorConfig *conf=nullptr);

    private slots:
        void cancelClicked();
        void okClicked();

    private:

        Context *context;
        RideItem *ride;
        DataProcessor *processor;
        DataProcessorConfig *config;
        QTextEdit *explain;
        QCheckBox *saveAsDefault;
        QPushButton *ok, *cancel;
};
#endif // _DataProcessor_h
