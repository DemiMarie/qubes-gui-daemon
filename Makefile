#
# The Qubes OS Project, http://www.qubes-os.org
#
# Copyright (C) 2010  Joanna Rutkowska <joanna@invisiblethingslab.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
#

RPMS_DIR=rpm/
VERSION := $(shell cat version)
MANDIR ?= /usr/share/man
LIBDIR ?= /usr/lib64

DIST_DOM0 ?= fc13

help:
	@echo "Qubes GUI main Makefile:" ;\
	    echo "make rpms                 <--- make all rpms and sign them";\
	    echo "make rpms-dom0            <--- create binary rpms for dom0"; \
	    echo "make rpms-vm              <--- create binary rpms for appvm"; \
	    echo; \
	    echo "make clean                <--- clean all the binary files";\
	    echo "make update-repo-current  <-- copy newly generated rpms to qubes yum repo";\
	    echo "make update-repo-current-testing <-- same, but for -current-testing repo";\
	    echo "make update-repo-unstable <-- same, but to -testing repo";\
	    echo "make update-repo-installer -- copy dom0 rpms to installer repo"
	    @exit 0;

dom0: gui-daemon/qubes-guid shmoverride/shmoverride.so shmoverride/X-wrapper-qubes \
		pulse/pacat-simple-vchan screen-layout-handler/watch-screen-layout-changes

gui-daemon/qubes-guid:
	(cd gui-daemon; $(MAKE))

shmoverride/shmoverride.so:
	(cd shmoverride; $(MAKE) shmoverride.so)

shmoverride/X-wrapper-qubes:
	(cd shmoverride; $(MAKE) X-wrapper-qubes)
	
pulse/pacat-simple-vchan:
	$(MAKE) -C pulse pacat-simple-vchan

screen-layout-handler/watch-screen-layout-changes:
	$(MAKE) -C screen-layout-handler watch-screen-layout-changes

install:
	install -D gui-daemon/qubes-guid $(DESTDIR)/usr/bin/qubes-guid
	install -m 0644 -D gui-daemon/qubes-guid.1 $(DESTDIR)$(MANDIR)/man1/qubes-guid.1
	install -D pulse/pacat-simple-vchan $(DESTDIR)/usr/bin/pacat-simple-vchan
	install -D pulse/pacat-control-api.xml $(DESTDIR)/usr/share/dbus-1/interfaces/org.qubesos.Audio.xml
	install -D -m 0644 pulse/org.qubesos.Audio.conf $(DESTDIR)/etc/dbus-1/system.d/org.qubesos.Audio.conf
	install -D shmoverride/X-wrapper-qubes $(DESTDIR)/usr/bin/X-wrapper-qubes
	install -D shmoverride/shmoverride.so $(DESTDIR)$(LIBDIR)/shmoverride.so
	install -D gui-daemon/guid.conf $(DESTDIR)/etc/qubes/guid.conf
	install -D gui-daemon/qubes-localgroup.sh $(DESTDIR)/etc/X11/xinit/xinitrc.d/qubes-localgroup.sh
	install -D gui-daemon/qubes.ClipboardPaste.policy $(DESTDIR)/etc/qubes-rpc/policy/qubes.ClipboardPaste
	install -D screen-layout-handler/watch-screen-layout-changes $(DESTDIR)/usr/libexec/qubes/watch-screen-layout-changes
	install -D screen-layout-handler/qubes-screen-layout-watches.desktop $(DESTDIR)/etc/xdg/autostart/qubes-screen-layout-watches.desktop
	$(MAKE) -C window-icon-updater install

rpms: rpms-dom0 rpms-vm
	rpm --addsign rpm/x86_64/*$(VERSION)*.rpm
	(if [ -d rpm/i686 ] ; then rpm --addsign rpm/i686/*$(VERSION)*.rpm; fi)

rpms-vm:
	rpmbuild --define "_rpmdir rpm/" -bb rpm_spec/gui-vm.spec

rpms-dom0:
	rpmbuild --define "_rpmdir rpm/" -bb rpm_spec/gui-dom0.spec

tar:
	git archive --format=tar --prefix=qubes-gui/ HEAD -o qubes-gui.tar

clean:
	(cd common; $(MAKE) clean)
	(cd gui-common; $(MAKE) clean)
	(cd gui-daemon; $(MAKE) clean)
	(cd shmoverride; $(MAKE) clean)
	$(MAKE) -C pulse clean

