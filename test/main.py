def qsort(arr):
    if len(arr) == 0:
        return arr
    if len(arr) == 1:
        return arr
    if len(arr) == 2:
        return [min(arr[0], arr[1]), max(arr[0], arr[1])]
    pivot_index = len(arr) // 2
    pivot = arr[pivot_index]
    sub_left = []
    sub_right = []
    for i in range(0, len(arr)):
        if i == pivot_index:
            break
        if arr[i] <= pivot:
            sub_left.append(arr[i])
        if arr[i] > pivot:
            sub_right.append(arr[i])
    return [*qsort(sub_left), pivot, *qsort(sub_right)]


print(qsort([4, 5, 1, 0, 6]))
