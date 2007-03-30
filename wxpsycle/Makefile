wxpsycle : wxpsycle.o mainwindow.o bitmaps.o
	g++ -o $@ $^ `wx-config --libs --unicode=yes`

.cpp.o :
	g++ -c -o $@ $< -DDEBUG -g `wx-config --cppflags --unicode=yes`

wxpsycle.o : wxpsycle.cpp mainwindow.h bitmaps.h
mainwindow.o : mainwindow.cpp mainwindow.h bitmaps.h
bitmaps.o : bitmaps.cpp bitmaps.h

clean:
	rm *.o wxpsycle