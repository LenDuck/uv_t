
.phony = client server
all: client server


client: 
	make -C src_client


server: 
	echo "none"

clean:
	rm ../uv_t/bin/*
	make -C src_client clean