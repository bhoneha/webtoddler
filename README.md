# webtoddler
A Learning Project - simple and limited webcrawler

**Web Toddler** is a minimal C++ web crawler that downloads HTML pages and extracts links using `libcurl` and `Lexbor`. The crawler has many limitations. I made this to learn the basics of libcurl and parsing webpages using libraries like lexbor. I have also learnt how to implement a DFS(Depth-First-Search) algorithm to use it to extract links from the DOM tree.

---

##  Features

- Download HTML from a given URL using `libcurl`
- Parse and extract `<a href="...">` links using `Lexbor`
- Breadth-first crawling with depth and page limits
- Basic URL normalization (relative → absolute)

---

## ⚙ Requirements

- C++17 or later
- [libcurl](https://curl.se/libcurl/)
- [Lexbor](https://github.com/lexbor/lexbor)

---

