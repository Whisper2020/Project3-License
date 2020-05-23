def exgcd(a,b,c):
    if b==0:
        return (c//a,0)
    else:
        r=exgcd(b,a%b,c)
        return (r[1],r[0]-r[1]*(a//b))

print(exgcd(65537,998244352*1000000006,1),end="\n\n")
