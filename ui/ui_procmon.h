/********************************************************************************
** Form generated from reading UI file 'procmon.ui'
**
** Created by: Qt User Interface Compiler version 5.0.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROCMON_H
#define UI_PROCMON_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout_3;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QPushButton *btn_start;
    QPushButton *btn_stop;
    QPushButton *btn_load;
    QPushButton *btn_unload;
    QTableView *procmon;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(998, 646);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        gridLayout_3 = new QGridLayout(centralwidget);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        groupBox = new QGroupBox(centralwidget);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setFlat(false);
        groupBox->setCheckable(false);
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        btn_start = new QPushButton(groupBox);
        btn_start->setObjectName(QStringLiteral("btn_start"));

        gridLayout->addWidget(btn_start, 0, 1, 1, 1);

        btn_stop = new QPushButton(groupBox);
        btn_stop->setObjectName(QStringLiteral("btn_stop"));

        gridLayout->addWidget(btn_stop, 0, 2, 1, 1);

        btn_load = new QPushButton(groupBox);
        btn_load->setObjectName(QStringLiteral("btn_load"));

        gridLayout->addWidget(btn_load, 0, 0, 1, 1);

        btn_unload = new QPushButton(groupBox);
        btn_unload->setObjectName(QStringLiteral("btn_unload"));

        gridLayout->addWidget(btn_unload, 0, 3, 1, 1);


        horizontalLayout->addWidget(groupBox);


        verticalLayout->addLayout(horizontalLayout);

        procmon = new QTableView(centralwidget);
        procmon->setObjectName(QStringLiteral("procmon"));

        verticalLayout->addWidget(procmon);


        gridLayout_3->addLayout(verticalLayout, 0, 0, 1, 1);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        groupBox->setTitle(QString());
        btn_start->setText(QApplication::translate("MainWindow", "Start tracking", 0));
        btn_stop->setText(QApplication::translate("MainWindow", "Stop tracking", 0));
        btn_load->setText(QApplication::translate("MainWindow", "Load module", 0));
        btn_unload->setText(QApplication::translate("MainWindow", "Unload module", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROCMON_H
