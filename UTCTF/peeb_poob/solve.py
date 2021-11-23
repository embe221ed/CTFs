file = './peeb_poob'
content = open(file, 'rb').read()

data = content[0x500:0x940]
result = ""

def reverse(val):
    byte = hex(val, )[2:].rjust(2, '0')
    return byte[::-1]

for c in range(len(data)//2):
    current = [data[c*2], data[c*2+1]]
    # print(hex(current[0]), hex(current[1]))
    result += reverse(current[0]) + reverse(current[1])

text = bytearray.fromhex(result)

file = './beep_boop'
with open(file, 'wb') as binary:
    binary.write(content[:0x500])
    binary.write(text)
    binary.write(content[0x940:])

