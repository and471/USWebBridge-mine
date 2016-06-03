function RangeValueControl(container, label) {
	this.container = container;

	this.container.addClass("range-value-control");

	this.range = $("<input type='range'/>").appendTo($(container));
	this.range.rangeslider({polyfill: false});
	this.number = $("<input type='number'/>").appendTo($(container));

	this.range.change(this.onRangeChange.bind(this));
	this.number.change(this.onNumberChange.bind(this));

	this.changeCallback = null;
}

RangeValueControl.prototype.onRangeChange = function() {
	this.number.val(this.range.val());
	if (this.changeCallback) this.changeCallback();
}

RangeValueControl.prototype.onNumberChange = function() {
	this.range.val(this.number.val()).rangeslider('update', true);
	if (this.changeCallback) this.changeCallback();
}

RangeValueControl.prototype.change = function(cb) {
	this.changeCallback = cb;
}

RangeValueControl.prototype.setVal = function(val) {
	this.range.val(val).rangeslider('update', true);;
	this.number.val(val);
}

RangeValueControl.prototype.val = function() {
	return parseFloat(this.number.val(), 10);
}

RangeValueControl.prototype.setMinMax = function(min, max) {
	this.number.attr("min", min).attr("max", max);
	this.range.attr("min", min).attr("max", max).rangeslider('update', true);
}

function RangeControl(container, label) {
	this.container = container;
	this.container.addClass("range-control");

	this.range = $("<input type='range'/>").appendTo($(container));
	this.range.rangeslider({polyfill: false});
	this.range.on("input", this.onRangeChange.bind(this));

	this.changeCallback = null;
}

RangeControl.prototype.onRangeChange = function() {
	if (this.changeCallback) this.changeCallback();
}

RangeControl.prototype.change = function(cb) {
	this.changeCallback = cb;
}

RangeControl.prototype.setVal = function(val) {
	this.range.val(val).rangeslider('update', true);
}

RangeControl.prototype.val = function() {
	return parseFloat(this.range.val(), 10);
}

RangeControl.prototype.setMinMax = function(min, max) {
	this.range.attr("min", min).attr("max", max).rangeslider('update', true);;
}

RangeControl.prototype.setStep = function(step) {
	this.range.attr("step", step).rangeslider('update', true);
}