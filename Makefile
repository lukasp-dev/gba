################################################################################
# These are variables for the GBA toolchain build
# You can add others if you wish to
# ***** YOUR NAME HERE *****
################################################################################

PROGNAME = App

# 명시적으로 컴파일할 .o 파일들 설정
OFILES = gba.o font.o main.o \
         images/gt.o images/garbage.o images/gtngarbage.o \
         images/win.o images/garbagedog.o \
         images/football.o

################################################################################
# These are various settings used to make the GBA toolchain work
# DO NOT EDIT BELOW.
################################################################################

.PHONY: all
all: CFLAGS += $(CDEBUG) -I../shared
all: LDFLAGS += $(LDDEBUG)
all: $(PROGNAME).gba
	@echo "[FINISH] Created $(PROGNAME).gba"

include /opt/cs2110-tools/GBAVariables.mak

LDFLAGS += --specs=nosys.specs
CFLAGS += -Wstrict-prototypes -Wold-style-definition -Werror=vla

.PHONY: debug
debug: CFLAGS += $(CDEBUG) -I../shared
debug: LDFLAGS += $(LDDEBUG)
debug: $(PROGNAME).gba
	@echo "[FINISH] Created $(PROGNAME).gba"

$(PROGNAME).gba: clean $(PROGNAME).elf client
	@echo "[LINK] Linking objects together to create $(PROGNAME).gba"
	@$(OBJCOPY) -O binary $(PROGNAME).elf $(PROGNAME).gba

$(PROGNAME).elf: crt0.o $(GCCLIB)/crtbegin.o $(GCCLIB)/crtend.o $(GCCLIB)/crti.o $(GCCLIB)/crtn.o $(OFILES) libc_sbrk.o
	$(CC) -o $(PROGNAME).elf $^ $(LDFLAGS)

.PHONY: client
client: client.c
	@gcc -o client client.c

.PHONY: mgba
mgba: debug
	@./client ${PROGNAME}.gba
	@echo "[EXECUTE] Running emulator mGBA"

.PHONY: gdb
gdb: debug
	@./client ${PROGNAME}.gba gdb
	@sleep 1
	@echo "[EXECUTE] Attempting to connect to GDB server."
	@echo "          Make sure mGBA is already running on the host machine!"
	@gdb-multiarch -ex 'file $(PROGNAME).elf' -ex 'target remote host.docker.internal:2345'

.PHONY: submit
submit: clean
	@rm -f submission.tar.gz
	@tar --exclude="examples" --exclude="mGBA*" -czvf submission.tar.gz *

.PHONY: maze
maze: client
	@echo "[EXECUTE] Running emulator mGBA"
	@echo "          Please see emulator.log if this fails"
	@./client examples/maze/Maze.gba

.PHONY: mario
mario: client
	@echo "[EXECUTE] Running emulator mGBA"
	@echo "          Please see emulator.log if this fails"
	@./client examples/mario/Mario.gba

.PHONY: pong
pong: client
	@echo "[EXECUTE] Running emulator mGBA"
	@echo "          Please see emulator.log if this fails"
	@./client examples/pong/Pong.gba

.PHONY: sandtris
sandtris: client
	@echo "[EXECUTE] Running emulator mGBA"
	@echo "          Please see emulator.log if this fails"
	@./client examples/sandtris/Sandtris.gba

.PHONY: clean
clean:
	@echo "[CLEAN] Removing all compiled files"
	rm -f *.o *.elf *.gba *.log */*.o *.sav */*/*.sav client
