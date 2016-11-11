all: prod_cons producer consumer

prod_cons: main.c
	gcc main.c -o prod_cons -lpthread

producer: producer.c
	gcc producer.c -o producer

consumer: consumer.c
	gcc consumer.c -o consumer

clean:
	$(RM) prod_cons producer consumer *.txt
