
CC=gcc

CFLAGS=-O3 -Wall -g -pg `pkg-config --cflags freetype2`

LDFLAGS=

LIBS=-lfreetype

/tmp/output_actionbar.mp4 : RENDER_RES=1920x1080

/tmp/output_actionbar.mp4 : DURATION=5.0

/tmp/output_actionbar.mp4 : FPS=60

/tmp/output_actionbar.mp4 : actionbar
	@./$^ $(RENDER_RES) $(DURATION) $(FPS) | ffmpeg -f rawvideo -pix_fmt rgb48le -r $(FPS) -s $(RENDER_RES) -i - -vcodec libx264 -pix_fmt yuv420p -profile:v baseline -level 3.0 -y $@

actionbar : writefile.o actionbar.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIBS)

