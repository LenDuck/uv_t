
.phony = client server
all: client server


client: 
	make -C src_client


server: 
	make -C src_server

clean:
	rm ../uv_t/bin/*
	make -C src_client clean
	make -C src_server clean
