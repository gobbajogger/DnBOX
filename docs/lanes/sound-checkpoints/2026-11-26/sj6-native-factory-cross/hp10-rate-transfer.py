#!/usr/bin/env python3
"""Derived 48k HP10 reference for the independent signed Raw Main-only bench.

This is an external reference-rate helper, not a firmware filter/parameter
change. Its 44.1k seed is the scoped native HP10 pole polynomial from the
actual reference leaf. Leave the Crusher and final DC coefficients literal
for this capture. Never fit the poles or level to the audio under test.
"""
import cmath
import math
import numpy as np

NATIVE_SR = 44100
# Native recurrence: q += g * p; p += r * p0 + (x - q).
# Equivalent denominator: [1, -1-r+g, r], unit DC rejection.
NATIVE_B0 = float.fromhex("0x1.ffa2ap-1")
NATIVE_G = float.fromhex("0x1.10427p-19")
NATIVE_R = float.fromhex("0x1.ff4564p-1")

def hp10(rate):
    a = -1.0 - NATIVE_R + NATIVE_G
    b = NATIVE_R
    if rate==NATIVE_SR:
        head=NATIVE_B0
    elif rate==48000:
        z=(-a+cmath.sqrt(a*a-4*b))*0.5
        s=2*NATIVE_SR*(z-1)/(z+1)
        z=(1+s/(2*rate))/(1-s/(2*rate))
        a=-2*z.real
        b=abs(z)**2
        # Preserve unity at Nyquist as in the native RBJ HP10 kernel.
        head=(1-a+b)/4
    else:
        raise ValueError("Only the exact 44.1k reference and its validated 48k transfer are admitted")
    return np.array((head,-2*head,head)), np.array((1,a,b))

def rbj_comparator(rate,q):
    ph=2*math.pi*10/rate
    alpha=math.sin(ph)/(2*q)
    d=1+alpha
    head=(1+math.cos(ph))/(2*d)
    return np.array((head,-2*head,head)),np.array((1,-2*math.cos(ph)/d,(1-alpha)/d))

def response(shape,f,sr):
    b,a=shape
    z=np.exp(-2j*np.pi*f/sr)
    p=np.array((1,z,z*z))
    return np.dot(p,b)/np.dot(p,a)

if __name__=="__main__":
    f=65.40639132514966
    mistakes=(3.586215,1.809,1.208)
    for k,measured in enumerate(mistakes,1):
        new=response(hp10(48000),k*f,48000)
        old=response(rbj_comparator(48000,1/math.sqrt(2)),k*f,48000)
        change=float(np.angle(new/old,deg=True))
        assert abs(change+measured)<.001,(k,change,measured)
        print(k, "predicted H phase correction degrees", format(change,".6f"))
    b,a=hp10(44100)
    assert b[0]==NATIVE_B0 and a[2]==NATIVE_R
