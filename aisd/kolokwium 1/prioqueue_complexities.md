# Złożonosci pesymistyczne

| Priority Queue | Insert                                         | Max      | DeleteMax                                  |
| -------------- | ---------------------------------------------- | -------- | ------------------------------------------ |
| Heap           | O(log n)                                       | O(1)     | O(log N)                                   |
| Beap           | O(sqrt(n))                                     | O(1)     | O(sqrt(n))                                 |
| Leftist Heap   | O(log n)                                       | O(1)     | O(log n)                                   |
| Skew Heap      | <span style = "color: red">**O(n)**</span>     | O(1)     | <span style = "color: red">**O(n)**</span> |
| Binomial Queue | <span style = "color: red">**O(log n)**</span> | O(log n) | O(log n)                                   |
| Fibonacci Heap | O(1)                                           | O(1)     | <span style = "color: red">**O(n)**</span> |

# Złożoności zamortyzowane

| Priority Queue | Insert                                           | Max      | DeleteMax                                        |
| -------------- | ------------------------------------------------ | -------- | ------------------------------------------------ |
| Heap           | O(log n)                                         | O(1)     | O(log N)                                         |
| Beap           | O(sqrt(n))                                       | O(1)     | O(sqrt(n))                                       |
| Leftist Heap   | O(log n)                                         | O(1)     | O(log n)                                         |
| Skew Heap      | <span style = "color: green">**O(log n)**</span> | O(1)     | <span style = "color: green">**O(log n)**</span> |
| Binomial Queue | <span style = "color: green">**O(1)**</span>     | O(log n) | O(log n)                                         |
| Fibonacci Heap | O(1)                                             | O(1)     | <span style = "color: green">**O(log n)**</span> |
