TEMPLATE = subdirs
SUBDIRS = lkm_loader procmon

CONFIG += ordered

app.path = /media/HDD/Proyectos/procmon
app.files += procmon/procmon

INSTALLS += app
