import random
from Crypto.Util.number import *
from sympy import factorint
from math import sqrt

# flag=open("flag","rb").read()

def encrypt(msg, N, x):
    msg = bin(bytes_to_long(msg))[2:]
    ciphertexts = []

    for i in msg:
        while True:
            r = random.randint(1, N)
            if gcd(r, N) == 1:
                bin_r = bin(r)[2:]
                val = int(bin_r + i, 2)
                c = (pow(x, val, N) * r ** 2) % N
                ciphertexts.append(c)
                break
    return ciphertexts


def factors(n):
    return set(reduce(list.__add__, ([i, n//i] for i in range(1, int(n**0.5) + 1) if n % i == 0)))

N = 76412591878589062218268295214588155113848214591159651706606899098148826991765244918845852654692521227796262805383954625826786269714537214851151966113019

x = 72734035256658283650328188108558881627733900313945552572062845397682235996608686482192322284661734065398540319882182671287066089407681557887237904496283

# flag = (encrypt(flag,N,x))

# open("handout.txt","w").write("ct:"+str(flag)+"\n\nN:"+str(N)+"\n\nx:"+str(x))
cipher = 3266754124418328672247866726679755848701520371443746634280907120024788951726790644565764235065010905480462052367779604906344974859812257431783971523801
