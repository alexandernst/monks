require './procmon/lkm'
include LKM

if LKM::check('procmon')
	puts "Procmon's kernel module doesn't seem to be loaded."
	LKM::load('./procmon_kmodule/procmon.ko')
	sleep(2)
	LKM::start()
	sleep(2)
	LKM::stop()
	sleep(2)
	LKM::unload('procmon')
end

