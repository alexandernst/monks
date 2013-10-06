require 'open3'
require './procmon/ui/lkm'
include LKM

if LKM::check('procmon')
	puts "Procmon's kernel module doesn't seem to be loaded."
	LKM::load('./procmon.ko')
	LKM::start()

	puts "Starting procmon-viewer..."
	output = []
	i = 0
	Open3.popen2("./procmon-viewer") do |stdin, stdout, wait_thr|
		stdout.each do |line|
			i += 1
			output << line.chomp
			if i >= 500 then
				break
			end
		end
		%x(kill -9 #{wait_thr.pid})
	end
	puts "#{output.size} lines where catched by procmon-viewer."

	LKM::stop()
	LKM::unload('procmon')
end
