bin_PROGRAMS = lustre
lustre_SOURCES = blockdevice.cpp client.cpp cluster.cpp device.cpp disk.cpp \
		elevator.cpp elvdeadline.cpp elvnoop.cpp event.cpp filesystem.cpp lov.cpp lustre.cpp \
		message.cpp netdevice.cpp network.cpp nic.cpp node.cpp nrsbinheap.cpp nrsepoch.cpp \
		nrsfifo.cpp nrsrbtree.cpp obd.cpp osc.cpp osd.cpp ost.cpp peer.cpp processor.cpp \
		raid0device.cpp rbtree.cpp scheduler.cpp stat.cpp timer.cpp mdc.cpp mdt.cpp

# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
lustre_LDFLAGS = $(all_libraries) 
noinst_HEADERS = blockdevice.h client.h cluster.h device.h disk.h elevator.h \
		elvdeadline.h elvnoop.h event.h filesystem.h hash.h lov.h lustre.h message.h \
		netdevice.h network.h nic.h node.h nrsbinheap.h nrsepoch.h nrsfifo.h nrsrbtree.h \
		obd.h osc.h osd.h ost.h peer.h processor.h raid0device.h rbtree.h scheduler.h \
		stat.h timer.h mdc.h mdt.h
