# def outer(msg):
#     def inner():
#         print(msg)

#     return inner


# hello = outer("hello")
# hello()


def outer():
    def inner():
        print("hello")

    return inner


hello = outer()
hello()
