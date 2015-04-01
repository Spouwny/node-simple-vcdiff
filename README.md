# node-simple-vcdiff
A really simple wrapper around [open-vcdiff](http://code.google.com/p/open-vcdiff)

## Description

This wraps the simplest API of [Open-VCDiff](http://code.google.com/p/open-vcdiff) for node v0.12

## API

### Require

```javascript
var vcdiff = require('simple-vcdiff');
```

### Encode
Compute the difference between the dictionnary and the target, sending the result buffer to the callback:
```javascript
vcdiff.Encode(dictionary, target, callback);
```


	
### Decode
Apply a previously computed delta to the dictionnary, sending the result buffer to the callback:
```javascript
vcdiff.Decode(dictionary, delta, callback);
```

### Callback

```javascript
function (bufferOut) {}
```

## Sample

```javascript
var vcdiff = require('simple-vcdiff');

var dictionary = new Buffer('testing data.');
var target = new Buffer('testing dota is not the same.');

vcdiff.Encode(dictionary, target, function(bufferDelta) {
	vcdiff.Decode(dictionary, bufferDelta, function(bufferOut) {
		// bufferOut should be identical to target.
	});
});
```


## Install

  `npm install simple-vcdiff`


