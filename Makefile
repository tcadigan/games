GAME = Omega
CFLAGS = -O -g -Wall
LDFLAGS = -lcurses

SRCS = oabyss.c oaux1.c oaux2.c oaux3.c o.c ochar.c ocity.c ocom1.c ocom2.c \
       ocom3.c ocountry.c oeffect1.c oeffect2.c oeffect3.c oenv.c oetc.c \
       ofile.c ogen1.c ogen2.c oguild1.c oguild2.c ohouse.c oinv.c oitem.c \
       oitemf1.c oitemf2.c oitemf3.c olev.c ommelee.c ommove.c omon.c omove.c \
       omovef.c omspec.c omstrike.c omtalk.c opriest.c osave.c oscr.c osite1.c \
       osite2.c ospell.c otime.c otrap.c outil.c ovillage.c 

OBJS = oabyss.o oaux1.o oaux2.o oaux3.o o.o ochar.o ocity.o ocom1.o ocom2.o \
       ocom3.o ocountry.o oeffect1.o oeffect2.o oeffect3.o oenv.o oetc.o \
       ofile.o ogen1.o ogen2.o oguild1.o oguild2.o ohouse.o oinv.o oitem.o \
       oitemf1.o oitemf2.o otiemf3.o olev.o ommelee.o ommove.o omon.o omove.o \
       omovef.o omspec.o omstrike.o omtalk.o opriest.o osave.o oscr.o osite1.o \
       osite2.o ospell.o otime.o otrap.o outil.o ovillage.o

HDRS = odate.h odefs.h oglob.h oiinit.h ominit.h

all: $(GAME) crypt decrypt

$(GAME): $(OBJS) $(HDRS) Makefile
	$(CC) -o $(GAME) $(OBJS) $(LDFLAGS)

CRYPT_SRC = crypt.c
CRYPT_OBJ = crypt.o

crypt.o: crypt.c
	$(CC) -c $(CFLAGS) crypt.c

crypt: $(CRYPT_OBJ)
	$(CC) -o crypt $(CRYPT_OBJ)

DECRYPT_SRC = decrypt.c
DECRYPT_OBJ = decrypt.o

decrypt.o: decrypt.c
	$(CC) -c $(CFLAGS) decrypt.c

decrypt: $(DECRYPT_OBJ)
	$(CC) -o decrypt $(DECRYPT_OBJ)

clean:
	rm -rf crypt decrypt $(DECRYPT_OBJ) $(CRYPT_OBJ) $(GAME)

oabyss.o: oglob.h
oaux1.o: oglob.h
oaux2.o: oglob.h
oaux3.o: oglob.h
o.o: oglob.h
occmds.o: oglob.h
ochar.o: oglob.h
ocity.o: oglob.h
ocom1.o: oglob.h
ocom2.o: oglob.h
ocom3.o: oglob.h odate.h
ocountry.o: oglob.h
oeffect1.o: oglob.h
oeffect2.o: oglob.h
oeffect3.o: oglob.h
oenv.o: oglob.h
oetc.o: oglob.h
ofile.o: oglob.h
ogen1.o: oglob.h
ogen2.o: oglob.h
oguild1.o: oglob.h
oguild2.o: oglob.h
ohouse.o: oglob.h
oinv.o: oglob.h
oitem.o: oglob.h
oitemf1.o: oglob.h
oitemf2.o: oglob.h
oitemf3.o: oglob.h
olev.o: oglob.h
ommelee.o: oglob.h
ommove.o: oglob.h
omon.o: oglob.h
omove.o: oglob.h
omovef.o: oglob.h
omspec.o: oglob.h
omstrike.o: oglob.h
omtalk.o: oglob.h
opriest.o: oglob.h
osave.o: oglob.h
oscr.o: oglob.h
osite1.o: oglob.h
osite2.o: oglob.h
ospell.o: oglob.h
otime.o: oglob.h
otrap.o: oglob.h
outil.o: oglob.h
ovillage.o: oglob.h
oglob.h: odefs.h oextern.h
