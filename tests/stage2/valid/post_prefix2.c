int main(){
    int a = 150;
    int b = 75;
    int c = a++ + b++;
    int d = a-- + b--;
    int e = ++c * ++d;
    int f = --c * --d;
    f += e;
    e -= d;
    d *= c;
    c /= b;
    b ^= a;
    a &= b;
    b |= c;
    c  = d;
    d <<= 3;
    f >>= 2;
    return a + b - c * d + e + f;
}
