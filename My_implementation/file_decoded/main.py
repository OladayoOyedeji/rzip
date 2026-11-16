import math

print("Hello world")

n = int(input())

print(n)

def isprime(n):
    for i in range(2, int(math.sqrt(n))+1):
        print(i)
        if n % i == 0:
            return False

    return True

print(isprime(n))
