#ifndef PROCMON_H
#define PROCMON_H

#include "lkm_loaderiface.h"

#include <QMainWindow>
#include <QtDBus/QtDBus>

namespace Ui {
	class MainWindow;
}

class ProcmonUI : public QMainWindow{
	Q_OBJECT
	
	public:
        ProcmonUI(QWidget *parent = 0);
        ~ProcmonUI();

    public Q_SLOTS:
        void add_syscall_data(const QString &data);
		
	private:
        int LOADED[4]   = {0, 1, 0, 1};
        int STARTED[4]  = {0, 0, 1, 0};
        int STOPPED[4]  = {0, 1, 0, 1};
        int UNLOADED[4] = {1, 0, 0, 0};

        Ui::MainWindow *ui;
        LKM_LoaderIface *lkm_loaderiface;
        int get_loaded_state();
        int change_loaded_state(int state);
        int change_running_state(int state);
        void change_btns_state(int states[]);

};

#endif // PROCMON_H
