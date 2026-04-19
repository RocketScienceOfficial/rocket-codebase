DATALINK_DIR := ../../datalink

.PHONY: run datalink

src/sim/.datalink_stamp: $(wildcard $(DATALINK_DIR)/schemas/*.xml) $(wildcard $(DATALINK_DIR)/templates/*.*) $(DATALINK_DIR)/gen.py
	python $(DATALINK_DIR)/gen.py --lang python --outdir src/sim
	@mkdir -p src/sim
	touch $@

datalink: src/sim/.datalink_stamp

run: src/sim/.datalink_stamp
	python -m sim.main --config $(CONFIG)