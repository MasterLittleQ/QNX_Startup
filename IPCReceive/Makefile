ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

BUILDDIR=build-aarch64le
INSTALLDIR=${INSTALL_ROOT_nto}/aarch64le/usr/
CMAKE=$(realpath ../../qcom/qc-tools/cmake-3.13.4-Linux-x86_64/bin/cmake)

install:
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR); $(CMAKE) \
					-DCMAKE_TOOLCHAIN_FILE=qnx-aarch64-toolchain.cmake \
					-DCMAKE_INSTALL_PREFIX=$(INSTALLDIR) \
					-DCMAKE_EXPORT_COMPILE_COMMANDS=yes \
					../
	$(CMAKE) --build $(BUILDDIR) --target install

clean:
	$(RM_HOST) $(INSTALLDIR)bin/ZStudy
	$(RM_HOST) -r $(BUILDDIR)
	$(RM_HOST) $(INSTALLDIR)/etc/system/config/slm/SLM_ZStudy.xml

.PHONY: install clean
