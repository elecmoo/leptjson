objects = test.o leptjson.o
leptjson_test: $(objects)
	cc -o leptjson_test $(objects)

leptjson.o: leptjson.h
test.o: leptjson.h leptjson.o

clean:
	$(RM) leptjson_test $(objects)
.PHONY: clean