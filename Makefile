TITLE_COLOR = \033[33m
NO_COLOR = \033[0m

# when executing make, compile all exe's
all: sensor_gateway sensor_node file_creator

# When trying to compile one of the executables, first look for its .c files
# Then check if the libraries are in the lib folder
sensor_gateway : main.c connmgr.c datamgr.c sensor_db.c sbuffer.c lib/libdplist.so lib/libtcpsock.so
	@echo "$(TITLE_COLOR)\n***** CPPCHECK *****$(NO_COLOR)"
	cppcheck --enable=all --suppress=missingIncludeSystem main.c connmgr.c datamgr.c sensor_db.c sbuffer.c
	@echo "$(TITLE_COLOR)\n***** COMPILING sensor_gateway *****$(NO_COLOR)"
	gcc -c main.c      -Wall -std=c11 -Werror -DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=5 -o main.o      -fdiagnostics-color=auto
	gcc -c connmgr.c   -Wall -std=c11 -Werror -DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=5 -o connmgr.o   -fdiagnostics-color=auto
	gcc -c datamgr.c   -Wall -std=c11 -Werror -DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=5 -o datamgr.o   -fdiagnostics-color=auto
	gcc -c sensor_db.c -Wall -std=c11 -Werror -DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=5 -o sensor_db.o -fdiagnostics-color=auto
	gcc -c sbuffer.c   -Wall -std=c11 -Werror -DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=5 -o sbuffer.o   -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING sensor_gateway *****$(NO_COLOR)"
	gcc main.o connmgr.o datamgr.o sensor_db.o sbuffer.o -ldplist -ltcpsock -lpthread -o sensor_gateway -Wall -L./lib -Wl,-rpath=./lib  -fdiagnostics-color=auto

file_creator : file_creator.c
	@echo "$(TITLE_COLOR)\n***** COMPILE & LINKING file_creator *****$(NO_COLOR)"
	gcc file_creator.c -o build/file_creator -DDEBUG -Wall -fdiagnostics-color=auto

sensor_node : sensor_node.c lib/libtcpsock.so
	@echo "$(TITLE_COLOR)\n***** COMPILING sensor_node *****$(NO_COLOR)"
	gcc -c sensor_node.c -Wall -std=c11 -Werror -o sensor_node.o -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING sensor_node *****$(NO_COLOR)"
	gcc sensor_node.o -ltcpsock -o sensor_node -Wall -L./lib -Wl,-rpath=./lib -fdiagnostics-color=auto

# If you only want to compile one of the libs, this target will match (e.g. make liblist)
libdplist : lib/libdplist.so
libtcpsock : lib/libtcpsock.so

lib/libdplist.so : lib/dplist.c
	@echo "$(TITLE_COLOR)\n***** COMPILING LIB dplist *****$(NO_COLOR)"
	gcc -c lib/dplist.c -Wall -std=c11 -Werror -fPIC -o lib/dplist.o -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING LIB dplist< *****$(NO_COLOR)"
	gcc lib/dplist.o -o lib/libdplist.so -Wall -shared -lm -fdiagnostics-color=auto

lib/libtcpsock.so : lib/tcpsock.c
	@echo "$(TITLE_COLOR)\n***** COMPILING LIB tcpsock *****$(NO_COLOR)"
	gcc -c lib/tcpsock.c -Wall -std=c11 -Werror -fPIC -o lib/tcpsock.o -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING LIB tcpsock *****$(NO_COLOR)"
	gcc lib/tcpsock.o -o lib/libtcpsock.so -Wall -shared -lm -fdiagnostics-color=auto

# do not look for files called clean, clean-all or this will be always a target
.PHONY : clean clean-all run zip

clean:
	rm -rf *.o sensor_gateway sensor_node file_creator *~

clean-all: clean
	rm -rf lib/*.so

test-connmgr: main.c connmgr.c sbuffer.c lib/tcpsock.c
	mkdir -p build
	gcc -g -Wall -std=c11 -Werror -DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=5 main.c connmgr.c sbuffer.c lib/tcpsock.c -o build/test-connmgr     -fdiagnostics-color=auto

test-datamgr: main.c datamgr.c sbuffer.c lib/tcpsock.c
	mkdir -p build
	gcc -g -Wall -std=c11 -Werror -DSET_MIN_TEMP=15 -DSET_MAX_TEMP=20 -DTIMEOUT=5 main.c datamgr.c sbuffer.c lib/dplist.c -o build/test-datamgr     -fdiagnostics-color=auto

test-datamgr-connmgr: main.c datamgr.c connmgr.c sbuffer.c lib/tcpsock.c
	mkdir -p build
	gcc -g -Wall -std=c11 -Werror -DSET_MIN_TEMP=15 -DSET_MAX_TEMP=2 -DTIMEOUT=5 main.c datamgr.c connmgr.c sbuffer.c lib/tcpsock.c lib/dplist.c -o build/test-datamgr-connmgr     -fdiagnostics-color=auto

test-storagemgr: main.c sensor_db.c sbuffer.c
	rm -f data.csv
	mkdir -p build
	gcc -g -Wall -std=c11 -Werror -DSET_MIN_TEMP=15 -DSET_MAX_TEMP=20 -DTIMEOUT=5 main.c sensor_db.c sbuffer.c -o build/test-storagemgr     -fdiagnostics-color=auto

test-storagemgr-connmgr: main.c sensor_db.c connmgr.c sbuffer.c lib/tcpsock.c
	rm -f data.csv
	mkdir -p build
	gcc -g -Wall -std=c11 -Werror -DSET_MIN_TEMP=15 -DSET_MAX_TEMP=20 -DTIMEOUT=5 main.c sensor_db.c connmgr.c sbuffer.c lib/tcpsock.c -o build/test-storagemgr-connmgr     -fdiagnostics-color=auto

test-connmgr-datamgr-storagemgr: main.c sensor_db.c connmgr.c datamgr.c sbuffer.c lib/tcpsock.c
	rm -f gateway.log
	rm -f data.csv
	mkdir -p build
	gcc -g -Wall -std=c11 -Werror -DSET_MIN_TEMP=15 -DSET_MAX_TEMP=10 -DTIMEOUT=5 main.c sensor_db.c connmgr.c datamgr.c sbuffer.c lib/tcpsock.c lib/dplist.c -o build/test-connmgr-datamgr-storagemgr     -fdiagnostics-color=auto

test-datamgr-storagemgr: main.c sensor_db.c datamgr.c sbuffer.c lib/tcpsock.c
	rm -f data.csv
	mkdir -p build
	gcc -g -Wall -std=c11 -Werror -DSET_MIN_TEMP=15 -DSET_MAX_TEMP=20 -DTIMEOUT=5 main.c sensor_db.c datamgr.c sbuffer.c lib/tcpsock.c lib/dplist.c -o build/test-datamgr-storagemgr     -fdiagnostics-color=auto

run : all
	./sensor_gateway 1258&
	sleep 1
	./sensor_node 1 4 127.0.0.1 1258&
	./sensor_node 15 4 127.0.0.1 1258&
	./sensor_node 21 4 127.0.0.1 1258&
	sleep 20
	killall sensor_node&
	sleep 25





zip:
	zip lab_final.zip main.c connmgr.c connmgr.h datamgr.c datamgr.h sbuffer.c sbuffer.h sensor_db.c sensor_db.h config.h lib/dplist.c lib/dplist.h lib/tcpsock.c lib/tcpsock.h