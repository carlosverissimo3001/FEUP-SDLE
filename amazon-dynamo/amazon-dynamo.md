<h1> Dynamo: Amazon's Highly Available Key-value Store </h1>

> [http://www.allthingsdistributed.com/files/amazon-dynamo-sosp2007.pdf](http://www.allthingsdistributed.com/files/amazon-dynamo-sosp2007.pdf)

- [Abstract](#abstract)
- [1. Introduction](#1-introduction)
- [2. Background](#2-background)
  - [2.1 System Assumptions and Requirements](#21-system-assumptions-and-requirements)
  - [2.2 Service Level Agreements (SLAs)](#22-service-level-agreements-slas)
  - [2.3 Design Considerations](#23-design-considerations)
- [3. Related Work](#3-related-work)
  - [3.1 Peer-to-Peer (P2P) Systems](#31-peer-to-peer-p2p-systems)
  - [3.2 Distributed File Systems and Databases](#32-distributed-file-systems-and-databases)
  - [3.3 Discussion](#33-discussion)
- [4. System Architecture](#4-system-architecture)
  - [4.1 System Interface](#41-system-interface)
  - [4.2 Partitioning Algorithm](#42-partitioning-algorithm)
  - [4.3 Replication](#43-replication)
  - [4.4 Data Versioning](#44-data-versioning)
  - [4.5 Execution of get() and put() Operations](#45-execution-of-get-and-put-operations)
  - [4.6 Handling Failures: Hinted Handoff](#46-handling-failures-hinted-handoff)
  - [4.7 Handling Permanent Failures: Replica Synchronization](#47-handling-permanent-failures-replica-synchronization)
  - [4.8 Membership and Failure Detection](#48-membership-and-failure-detection)
    - [4.8.1 Ring Membership](#481-ring-membership)
    - [4.8.2 External Discovery](#482-external-discovery)
    - [4.8.3 Failure Detection](#483-failure-detection)
  - [4.9 Adding and Removing Storage Nodes](#49-adding-and-removing-storage-nodes)
- [5. Implementation](#5-implementation)
- [6. Experiences and Lessons Learned](#6-experiences-and-lessons-learned)
  - [6.1 Balancing Performance and Durability](#61-balancing-performance-and-durability)
  - [6.2 Ensuring Uniform Load Distribution](#62-ensuring-uniform-load-distribution)
  - [6.3 Divergent Versions: When and How Many?](#63-divergent-versions-when-and-how-many)
  - [6.4 Client-driven or server-driven coordination?](#64-client-driven-or-server-driven-coordination)
  - [6.5 Balancing background vs foreground tasks](#65-balancing-background-vs-foreground-tasks)
  - [6.6 Discussion](#66-discussion)
- [7. Conclusions](#7-conclusions)

## Abstract

One of the biggest challenges faced by Amazon is reliability at a large sacle. The Amazon platform is build on top of tens of thousands of servres and network components, in different areas of the work.

At this scale, it is not uncommom for small or large components to fail The following text file summarizes the design and implementation of Dynamo, a highly available key-value storage system that some of Amazon's core services use to provide an always-on experience.

**Dynamo sacrifices consistency under certain failure scenarios**.

It makes use of object versioning:

> (Object) Versioning allows multiple revisions of a single object to exist.

It also uses an "application-assisted conflict resolution mechanism" in situations where versioning is not sufficient.

## 1. Introduction

> Reliability and scalability of a system are dependant on how its application state is managed.

Amazon utilizes Dynamo to oversee the operational state of services that demand high availability and necessitate precise control over the trade-offs between availability, consistency, cost-efficiency, and performance.

Dynamo amalgamates established methodologies to achieve scalability and availability, specifically through data partitioning and replication. It employs consistent hashing for both data partitioning and replication, while ensuring data consistency through object versioning.

To maintain consistency across diverse replicas, Dynamo utilizes a quorum-like approach and a decentralized replica synchronization protocol. It incorporates a distributed failure detection and membership protocol based on gossip mechanisms.

Dynamo is designed as a fully decentralized system, requiring minimal manual administration. The addition or removal of storage nodes from Dynamo can be executed seamlessly, without necessitating manual partitioning or data redistribution.

## 2. Background

Relational databases are often ill-suited for numerous common use cases at Amazon. Many of these services primarily involve straightforward data storage and retrieval based on a primary key, eliminating the need for the complex relational queries that traditional RDBMSs are specifically designed to handle. Utilizing relational databases in such scenarios can be excessive, demanding costly hardware and skilled personnel for operation.

Furthermore, the replication techniques typically employed by RDBMSs tend to prioritize data consistency over availability, which may not align with the requirements of certain Amazon services.

Scaling out RDBMSs and applying load-balancing techniques to distribute workloads across multiple machines is often a challenging task.

In contrast, Dynamo offers a simple key/value interface, ensuring high availability and defining a clear consistency window. It excels in its scalability, providing an efficient approach to handling increased workloads.

Each service employing Dynamo operates its own instances, allowing for tailored system adjustments to meet the unique demands of the service.

### 2.1 System Assumptions and Requirements

**Query Model** - simple read/write ops. to a data item uniquely identified by a key.

How is state store? - as binary objects, i.e. blobs, identified by unique keys.

No operations span multiple data items, so the need for a relational schema is eliminated.

Dynamo targets applications that need to store relatively small objects (typically less than 1 MB).

**ACID Properties** - set of properties that guarantee that database transactions are processed reliably.
Experience at Amazon has shown that data stores that provide ACID guarantees tend to have poor availability. Dynamo targets applications that operate trade sacrifice consistency if it results in high availability.

Dynamo does NOT provide isolation guarantees and allows for only single-key updates.

**Efficiency** - 99.9th percentile of the distribution in regards to latency is a requirement for Amazon services. Since state acess plays such crucial role in the performance of a service, Dynamo must be able to scale out to meet the performance requirements regardless of the workload.

Services must configure their Dynamo instances to meet their performance requirements. The tradeofs are in performance, cost efficiency, and durability guarantees.

**Other Assumptions** - Dynamo is used only by Amazon services and its operational environment is assumed to be non-hostile.

### 2.2 Service Level Agreements (SLAs)

Clients and services engage in formal SLA contracts, specifying system-related characteristics such as the client's expected request rate distribution and the expected service latency under various conditions. For example, a simple SLA might guarantee a response within 300ms for 99.9% of requests at a peak client load of 500 requests per second.

Amazon's approach to SLAs is unique in that it is expressed and measured at the 99.9th percentile of the distribution, rather than relying on average or median metrics. This ensures a consistent high-quality experience for all customers, including those with more extensive usage.

Many papers in the industry report on averages, but Amazon's engineering efforts concentrate on controlling performance at the 99.9th percentile. The choice of the 99.9th percentile is based on a cost-benefit analysis that demonstrated significant cost increases to improve performance even further. Amazon's practical experiences have shown that this approach provides a superior overall customer experience compared to systems that focus on mean or median performance.

The chapter highlights the role of storage systems in establishing an SLA, especially when business logic is lightweight. In this context, state management becomes a crucial component of an SLA. The design of Dynamo, as discussed in the paper, prioritizes giving services control over their system properties, allowing them to make trade-offs between functionality, performance, and cost-effectiveness.

### 2.3 Design Considerations

Data replication algorithms used in comercial systems usually perform synchronous replication, which guarantees consistency, but at the cost of the availability of the data under certain failure scenarios.

For example, the data is made unavailable until the system is certain that the data is 100% consistent across all replicas. This is not acceptable for Amazon services, which require high availability and low latency.

**Strong consistency and high availability are at odds with each other**.

If the system is prone to network/server failures, availability can be improved by relaxing consistency requirements. The challenge is that this can lead to data divergence, which can be difficult to resolve.
And two new difficulties arise:

- When should the system fix the data divergence?
- Who should resolve the data divergence?

Dynamo is designed to be an eventually consistent data store. The updates will reach all replicas, eventually.

It is important to decide when to resolve the data divergence. Should this be done during reads or writes?

- Many traditional systems resolve data divergence during writes. In such systems, writes may fail if the data store cannot reach all replicas.
- Dynamo prefers an 'always writeable' approach. Amazon cannot afford to reject customer updates, i.e, adding/removing items from the shopping cart. The complexity is thus pushed to the read ops.

But who should resolve the data divergence? The client or the server?

- If conflict resolution is done by the data store (server), choices are limited. Only policies such as 'last write wins' can be implemented.
- The applicaction (client) can implement more sophisticated conflict resolution policies since it is aware of the data semantics.

Other key design considerations:

| Design Consideration | Description |
| -------------------- | ----------- |
| **Incremental scalability** | Dynamo must be able to scale out one storage host at a time, with minimal impact on both operators and the system itself. |
| **Symmetry** | Every node in Dynamo should have the same set of responsibilities as its peers; there should be no distinguished node or nodes that take special roles or extra set of responsibilities. |
| **Decentralization** | An extension of symmetry, Dynamo should be decentralized and should minimize the need for manual administration. It should be able to automatically partition, distribute, and recover data in the face of node failures. |
| **Heterogeneity** | Dynamo should be able to exploit heterogeneity in the infrastructure it runs on. It should be able to use all the available resources in the infrastructure, and should scale well under heterogeneous loads. |

## 3. Related Work

Following sectiosn discuss related work in the areas of peer-to-peer systems, distributed file systems, and distributed databases.

### 3.1 Peer-to-Peer (P2P) Systems

1st Gen P2P systems -> Freenet, Gnutella : Unstructured P2P systems.

2nd Gen P2P systems -> Chord, Pastry : Structured P2P systems, i.e, use routing mechanisms to ensure queries
are answered within a small number of hops. To reduce latency introduced by multi-hop routing, some P2P systems employ O(1) routing where each peer maintains a small routing table so it can route requests to the appropriate peer within a constant number of hops.

Some storage systems like Oceanstore and PAST are buld on top of these routing overlays.

**Oceanstore** : global, transactional, persistent storage service that supports serialized updates on widely replicated data. To allow for concurrent updates while avoid locking problems, it uses an update model based on conflict resolution. Conflict resolution was introuced to reduce the number of transaction that abort. Conflict resolution is done by processing a series of updates, choosing a total order amongst them and them applying them atomically in that order.

On the other hand, **PAST** provides an abstraction layer on top of Pastry.

### 3.2 Distributed File Systems and Databases

<!--  WRITE LATER, NOT THAT IMPORTANT RN -->

### 3.3 Discussion

Dynamo differs from the aforementioned systems in the following ways:

- Dynamo's main target are applications that need an 'always writeable' data store where, even under failures, updates never fail. This is CRUCIAL for many Amazon services.
- Secondly, Dynamo is build for an environment within a single administrative domain, i.e, Amazon's data centers, where ALL nodes are assumed to be trusted.
- Application that use Dynamo, do NOT require technical support for hierarchical namespaces, or complex relational schemas. Dynamo is a simple key/value store.
- Dynamo is build for latency sensitive apps that require, at least, 99.9% of read/write ops to be served within a few hundred milliseconds. To meet these, the developers had to ditch routing request through many nodes (like Pastry and Chord do) instead, each nodes maintains enough routing info locally to route a request to the appropriate node directly. They call it 'zero-hop DHT routing'. DHT stands for Distributed Hash Table.

> Read more about zero-hop DHT here: <http://datasys.cs.iit.edu/reports/2012_Qual-IIT_ZHT.pdf>

## 4. System Architecture

The architecture is complex, as expected of a production setting. In addiction to the data persistent component, the following scalable and robust components are also present:

- load balancing
- membership and failure detection
- failure recovery
- replica synchronization
- overload handling
- state transfer
- concurrency and job scheduling
- request marshalling
- request routing
- system monitoring and alarming
- configuration management

The paper only discusses [partitioning](#42-partitioning-algorithm), [replication](#43-replication), [membership and failure detection](#48-membership-and-failure-detection), [failure handling](#46-handling-failures-hinted-handoff) and [scaling](#49-adding-and-removing-storage-nodes).

**Summary of Techniques Used by Dynamo**: The following table summarizes the techniques used by Dynamo to achieve its design goals.

| Problem | Technique | Advantage |
| :-------: | :---------: | :---------: |
| **Partitioning** | Consistent Hashing | Incremental Scalability |
| **High-Availability for writes** | Vector Clocks with reconciliation during reads | Version size is decoupled from update rates |
| **Handling temporary failures** | Sloppy Quorum and hinted handoff | Provides high-availability and durability guarantees when some of the replicas are not available |
| **Recovering from permanent failures** | Anti-entropy using Merkle trees | Synchronizes divergent replicas in the background |
| **Membership and failure detection** | Gossip-based membership protocol | Preserves symmetry and avoids having a centralized registry for storing membership and node liveness information |

### 4.1 System Interface

Dynamo stores objects associated with a key through a simple interface, it exposes two operations:

- `get(key)`: Given a key, locates the object replicas associated with the key in the storage systems and returns a single object, or a list of objects with conflicting versions along with a context.
- `put(key, context, object)`: Determines where the replicas of the object should be placed based on the associated key and writes replicas to disk. `context` encodes metadata about the object that is opaque to the caller and includes info such as the version of the object. This context is shared along with the object so that the systems verifies the validity of the context object supplied by the caller.

Both key and object supplied by the caller are treated by Dynamo as an opaque array of bytes.
It applies a MD5 hash on the key to generate a 128-bit ID, which determines storage nodes that are responsible for serving the key.

> By opaque, they mean that Dynamo does not have any knowledge about the structure of the key or the object. It doesnt try to interpret or manipulate the content, and just stores and retrieves it as an opaque array of bytes.

### 4.2 Partitioning Algorithm

> To remember: One of the key design requirements is that it must scale incrementally, i.e, adding a new node to the system should be easy and should not require any manual partitioning or redistribution of data.

The Dynamo's partitioning scheme relies on consistent hashing to distribute the load across multiple storage hosts.

The output range of an hash function in consistent hashing is treated as a fixed circular space or "ring".
The idea is that the largest hash value wraps around to the smallest hash value, forming a continuous loop.

> Imagine this "ring" as a circle where hash values are evenly distributed along the circumference. Each node in the system is also assigned a position on this ring, determined by hashing its identifier or address. Data is then associated with a position on the ring based on its hash value. When you want to find which node should be responsible for a particular piece of data, you hash the data's key, find its position on the ring, and locate the nearest node in a clockwise direction. This ensures that data distribution is fairly even and allows for efficient load balancing.

This way, each node is responsible for an area in the ring between it and its predecessor node. The main advantage is that the removal of a node only affects its imediate neighbors and the other nodes remain unaffected.

Some challanges arise when using the basic consistent hashing algorithm:

- the random position assignment of nodes in the ring leads to non-uniform data and load distribution.
- the basic algorithm is oblivous to heterogenity in the performance of nodes.
  - to adress this, dynamo uses a variant of consistent hashing: instead of mapping a node to a single point in the circle, each node gets assigned to multiple points in the ring. Thus, dynamos uses "virtual nodes", that look like normal nodes in the system, but each node can be responsible for more than one virtual node.

Using virtual nodes brings three main advantages:

- if a node becomes unavailable, the load handled by this node is evenly dispersed across the remaining available nodes.
- when a node becomes available again, or a new node is added to the system, the newly available node accepts a roughly equivalent amount of load from each of the other available nodes.
- the number of virtual nodes that a node is responsible for can be decided based on its capacity, addressing the heterogeneity issue.

### 4.3 Replication

To ensure high availability and durability, Dynamo employs data replication. Each data item is replicated on multiple hosts, with the number of replicas, denoted as N, being a configurable parameter for each instance. Here's a summary of the key points:

**Replication Configuration**: Each key, denoted as "k," is assigned to a coordinator node. The coordinator is responsible for replicating data items falling within its designated range. Replication is carried out by storing each key locally at the coordinator and additionally replicating it at the N-1 clockwise successor nodes in the ring.

**Responsibility Zones**: This replication scheme divides the ring into regions of responsibility for each node. Each node is responsible for the region of the ring between itself and its Nth predecessor. For example, in the figure below, node B replicates key k at nodes C and D, along with its local storage. Node D stores keys within the ranges (A, B], (B, C], and (C, D].

<p align="center">
  <img src="images/replication.png" alt="Replication" width="500"/>

  <p align="center"><i>Figure 1: Responsability Zones</i></p>
</p>

**Preference List**: The list of nodes responsible for storing a particular key is referred to as the "preference list." The system is designed to enable every node to determine the nodes that should be on this list for any given key. This facilitates efficient data retrieval.

**Handling Node Failures**: To account for node failures and ensure availability, the preference list contains more than N nodes. This redundancy helps maintain data accessibility even in the presence of node failures.

**Virtual Nodes**: With the use of virtual nodes, it's possible that the first N successor positions for a key may be owned by fewer than N distinct physical nodes. In other words, a single physical node may hold multiple positions in the preference list. This design choice allows for more flexible load distribution and resilience to node failures.

The key takeaway is that Dynamo's replication strategy is designed for high availability, durability, and efficient data retrieval. It utilizes preference lists to ensure that data is stored redundantly and accessible even in the event of node failures, while virtual nodes enhance flexibility and load balancing.

### 4.4 Data Versioning

### 4.5 Execution of get() and put() Operations

### 4.6 Handling Failures: Hinted Handoff

### 4.7 Handling Permanent Failures: Replica Synchronization

### 4.8 Membership and Failure Detection

#### 4.8.1 Ring Membership

#### 4.8.2 External Discovery

#### 4.8.3 Failure Detection

### 4.9 Adding and Removing Storage Nodes

## 5. Implementation

## 6. Experiences and Lessons Learned

### 6.1 Balancing Performance and Durability

### 6.2 Ensuring Uniform Load Distribution

### 6.3 Divergent Versions: When and How Many?

### 6.4 Client-driven or server-driven coordination?

### 6.5 Balancing background vs foreground tasks

### 6.6 Discussion

## 7. Conclusions
