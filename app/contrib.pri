INCLUDEPATH     += $$PROJECT_ROOT/contrib/fit/include \
				   $$PROJECT_ROOT/contrib/miniz/include \
				   $$PROJECT_ROOT/contrib/aes/include

LIBS            += -L$$PROJECT_ROOT/contrib/fit/$$BUILD_POSTFIX \
				   -L$$PROJECT_ROOT/contrib/miniz/$$BUILD_POSTFIX \
				   -L$$PROJECT_ROOT/contrib/aes/$$BUILD_POSTFIX

LIBS            += -lfit \
				   -lminiz \
				   -laes
