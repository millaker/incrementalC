int foo();
int bar(int a);

int main(){
    int var = 1;
    while(var < 50)
        var = bar(var);
    return var + foo();
}

int foo(){
    return 3;
}

int bar(int a){
    return a * 2;
}
