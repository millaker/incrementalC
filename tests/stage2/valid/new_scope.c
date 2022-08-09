int main(){
    int a = 5;
    {
        a = 7;
        int a = 13;
        a = 15;
    }
    return a;
}
