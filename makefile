TARGET:= comol


.PHONY:	all clean

all: $(TARGET)

$(TARGET): 
	$(MAKE) -C src/ ../comol

clean:
	$(MAKE) -C src/ clean
	rm -f $(TARGET)

.PHONY: clean