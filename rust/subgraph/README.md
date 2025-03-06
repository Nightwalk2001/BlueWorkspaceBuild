# 频繁子图挖掘 - _RUST 实现_

## 实现算法

- [x] gSpan
- [ ] ……

### gSpan

gSpan 是一个频繁子图挖掘的算法，该算法在 Xifeng Yan 的 [gSpan](https://sites.sc.ucsb.edu/~xyan/software/gSpan.htm) 中进行了详细介绍。

#### 重要数据结构说明

- `Graph`: 图的临接边数据结构，包含一列 `Vertex`, 每个 `Vertex` 包含一列 `Edge`。变量 `trans` 记录了所有图。

  - `get_backward()`: 获取图中最右下节点到最右路径上的 Backward 边。
  - `get_forward_pure()`: 获取图中最右下节点引出的所有 Forward 边。
  - `get_forward_rm_path()`: 获取图中最右路径引出的所有 Forward 边。
  - `get_forward_edges()`: 获取图中某节点引次数的有效边 (a.label <= a.nei.label), 用于构造 `DFSCode`

- `DFSCode`: `DFSCode` 是一个图中所有边信息的序列，每条边上记录顶点 ID、顶点和边的 Label。该排列具有某种全序关系定义，因此在该全序关系上具有最小值，我们可只对具有最小值的 `DFSCode` 进行拓展挖掘。图同构等价于 `DFSCode` 相同。

  - `to_graph()`: 将 `DFSCode` 转换为 `Graph`。
  - `build_rm_path()`: 在 `DFSCode` 上获取最右路径，保存了最右路径上节点在 `DFSCode` 上的索引。

- `PrevDFS`: `PrevDFS` 的数据结构是一个链表，其本质代表了深度优先搜索中，`DFSCode` 的搜索栈中，每次传入一个 `Projected`，代表当前的 `DFSCode` 在所有原图中的投影（Projection）。由于每个 child DFSCode 都是在 parent DFSCode 上拓展而来，如果将每个图或每个图的 `DFSCode` 保存在搜索栈中就会浪费大量空间。因此当前栈中只保存增加的边，即 `PrevDFS.edge`，运行时根据 `PrevDFS.prev` 的链表指针向前寻找，即可构造出该 `DFSCode` 每一条边的添加顺序。

- `Projected`: `Projected` 最主要的作用是在栈中保存所有的 `PrevDFS`，他是一个 `PrevDFS` 的数组。在递归调用的子挖掘的搜索栈中，每次传入一个 `Projected`，代表当前的 `DFSCode` 在所有原图中的 “投影”（出现位置及每条边被添加的顺序），然后在所有原图中的每个出现位置上尝试拓展新边，构造出下一层的多个 `Projected`，然后对这些 `Projected` 依次递归调用子挖掘方法。

- `History`: 在递归函数中，根据当前 `DFSCode` 构造出的 `rmpath` 保存了最右路径上的节点索引，我们需要根据这些索引找到原图中最右路径上的边的指针。利用 `PrevDFS.prev` 的链表指针向搜索栈上方一个个寻找，可构造出整个图上的边被添加的顺序，该顺序与 `DFSCode` 中边的排列顺序相同。这样 `History.histories` 就恢复出了按照 `DFSCode` 的排列形式在对应出现位置上的所有边的指针，即可利用 `rmpath` 的索引信息之间定位到其出现位置上的边的指针。
  - `History.vertex` 和 `History.edge` 各是一个 HashSet，如果该点和边已经出现在 `DFSCode` 中，则相应位置加入集合中。

---

## 运行

### 参数

```rust
pub struct Config {
    pub input_source: InputSource,  // 输入源，可以是文件路径或 Vec<Graph>
    pub process_path: ProcessPath,  // 过程文件路径，debug 时传入，将打印搜索过程数据
    pub output_path: OutputPath,    // 输出文件路径，如果传入将输出结果到文件中
    pub output_type: OutType,       // 输出文件格式，支持 txt 和 json
    pub min_support: usize,         // 相同结构在不同图中出现的最小次数
    pub min_inner_support: usize,   // 相同结构在图内部中出现的最小次数
    pub min_vertices: usize,        // 子图最小节点数
    pub max_vertices: usize,        // 子图最大节点数
}
```

### 输入

#### 文件 json 格式

e.g. `json\lenet.json`

```json
{
  "name": "Lenet Graph",
  "nodes": {
    "x:12": {
      "name": "x:12",
      "opType": "ReLU",
      "input": ["x:10"]
    },
    "x:10": {
      "name": "x:10",
      "opType": "Conv2D",
      "input": ["x:7"]
    }
  },
  "parameterust": {}
}
```

#### Graph 格式

```rust
pub struct Graph {
    pub id: usize,
    pub name: String,
    pub edge_size: usize,
    pub directed: bool,
    pub vertices: Vec<Vertex>,
    pub vertex_name_label_map: HashMap<String, String>,
}
```

### 启动

```rust
// 使用示例
fn main() {
  println!("gSpan Subgraph Mining");
  println!("---------------------");

  let gspan_mining = GSpanMining;

  let mining_strategy = MiningContext::new(Box::new(gspan_mining));

  // 按文件路径启动
  // let config = Config::new(
  //   filename,
  //   Some("out-t-process.txt"),
  //   Some("out-t.txt"),
  //   OutType::TXT,
  //   1,
  //   2,
  //   1,
  //   10,
  // );

  // 按 Graph 启动
  let graph = Graph::graph_from_file(r#"json\lenet.json"#, true).unwrap();
  let config = Config::new(
    vec![graph],
    Some("out-t-process.txt"),
    Some("out-t.txt"),
    OutType::TXT,
    1,
    2,
    1,
    10,
  );

  match config {
    Ok(config) => {
      /* 用法一：结果写入文件，同时返回算法运行完成后的全部子图数据
      *     可以创建文件读取流，轮询监听，读取所有新数据
      */
      let total_result = mining_strategy.run(config);

      println!("{:?}", total_result);

      // 用法二：通过 channel 获取数据结果，数据通过 channel 一个一个返回
      // let receiver = mining_strategy.run_channel(config);

      // 从接收端读取消息
      // for received in receiver {
      //     println!("\n> Main | Received: {}\n> END", received);
      // }
    }
    Err(e) => eprintln!("Failed to create config: {:?}", e),
  }
}
```

### 输出

#### Output: txt 格式

```txt

t # 0 * btw(1) inn(2, 2) ttl(2)
v 0 BiasAdd
v 1 ReLU
v 2 MatMul
v 3 BiasAdd
e 0 1 BiasAdd ReLU <NIL>
e 1 2 ReLU MatMul <NIL>
e 2 3 MatMul BiasAdd <NIL>
$4| 0/18:26, 0/18:24, 0/18:20, 0/16:22
 e| 18:24/MatMul-<NIL>-18:26/BiasAdd
 e| 16:22/ReLU-<NIL>-18:24/MatMul
 e| 18:20/BiasAdd-<NIL>-16:22/ReLU
$4| 0/18:29, 0/18:31, 0/18:26, 0/16:27
 e| 18:29/MatMul-<NIL>-18:31/BiasAdd
 e| 16:27/ReLU-<NIL>-18:29/MatMul
 e| 18:26/BiasAdd-<NIL>-16:27/ReLU

t # 1 * btw(1) inn(2, 2) ttl(2)
v 0 Conv2D
v 1 ReLU
v 2 MaxPool
e 0 1 Conv2D ReLU <NIL>
e 1 2 ReLU MaxPool <NIL>
$3| 0/x:12, 0/x:13, 0/x:10
 e| x:12/ReLU-<NIL>-x:13/MaxPool
 e| x:10/Conv2D-<NIL>-x:12/ReLU
$3| 0/x:3, 0/x:5, 0/x:7
 e| x:5/ReLU-<NIL>-x:7/MaxPool
 e| x:3/Conv2D-<NIL>-x:5/ReLU
```

#### Output: json 格式

```json
[{
  "between_sup": 1,
  "inner_min_sup": 2,
  "inner_max_sup": 2,
  "total": 2,
  "structure": {
    "tid": 0,
    "vertices": [
      { "name": "0", "label": "BiasAdd" },
      { "name": "1", "label": "ReLU" },
      { "name": "2", "label": "MatMul" },
      { "name": "3", "label": "BiasAdd" }
    ],
    "edges": [
      {
        "from": "0",
        "to": "1",
        "from_label": "BiasAdd",
        "to_label": "ReLU",
        "e_label": "<NIL>"
      },
      ...
    ]
  },
  "instances": [
    {
      "node_num": 4,
      "node_ids": [
        { "gid": 0, "nid": "18:29" },
        { "gid": 0, "nid": "18:31" },
        { "gid": 0, "nid": "18:26" },
        { "gid": 0, "nid": "16:27" }
      ],
      "edges": [
        {
          "from": "18:29",
          "to": "18:31",
          "from_label": "MatMul",
          "to_label": "BiasAdd",
          "e_label": "<NIL>"
        },
        ...
      ]
    },
    ...
  ]
}]
```

#### Process: 过程文件 txt 格式

> 过程文件没有 json 格式

```txt
t # 0 * btw(1) inn(3, 3) ttl(3) // 单节点重复
v result_0 BiasAdd
18:26,18:20,18:31
...
-------                         // 多节点重复
t # 5 * btw(1) inn(2, 2) ttl(2)
v 0 BiasAdd
v 1 ReLU
e 0 1 BiasAdd ReLU <NIL>
...
```