COLORVIEW = colorview
PIXCONVERT = pixconvert
PIXEDIT = pixedit
CXXFLAGS = `sdl2-config --cflags` -Wall
LDFLAGS = `sdl2-config --libs` -lSDL2_image

COLORVIEW_OBJS = colorview.o
COLORVIEW_CPPFILES = colorview.cpp

PIXCONVERT_OBJS = pixconvert.o
PIXCONVERT_CPPFILES = pixconvert.cpp

PIXEDIT_OBJS = pixedit.o
PIXEDIT_CPPFILES = pixedit.cpp

all: $(COLORVIEW) $(PIXCONVERT) $(PIXEDIT)

$(COLORVIEW): $(COLORVIEW_OBJS)
	$(CXX) $(CXXFLAGS) $(COLORVIEW_OBJS) -o $(COLORVIEW) $(LDFLAGS)

$(PIXCONVERT): $(PIXCONVERT_OBJS) $(PIXCONVERT_HDRS)
	$(CXX) $(CXXFLAGS) $(PIXCONVERT_OBJS) -o $(PIXCONVERT) $(LDFLAGS)

$(PIXEDIT): $(PIXEDIT_OBJS)
	$(CXX) $(CXXFLAGS) $(PIXEDIT_OBJS) -o $(PIXEDIT) $(LDFLAGS)

clean:
	rm -f $(COLORVIEW_OBJS) $(COLORVIEW)
	rm -f $(PIXCONVERT_OBJS) $(PIXCONVERT)
	rm -f $(PIXEDIT_OBJS) $(PIXEDIT)

cppfiles: $(COLORVIEW_CPPFILES) $(PIXCONVERT_CPPFILES) $(PIXEDIT_CPPFILES)
