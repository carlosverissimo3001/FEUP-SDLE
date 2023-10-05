# Large Scale Distributed Systems - 4th Class

> 4th October 2023

## Qourom-Consensous and Replicated Abstract Data Types

**Quorom** for an operation is any set of replicas whose cooperation is sufficient to perform the operation.

- When executing an operation, a client:
  - reads from an **initial quorum**
  - writes to a **final quorum**
  - For e.g, in the read op., the client must read from some set of replicas, but its final quorom is empty.
  - A **quorom for an operation** is any set of replicas that includes **both an initial and final quorom**.
  - Assuming all replicas are considered equals, a quorom may be represented by a a pair (m,n), whose elements are the sizes of its initial and final quoroms, respectively.

Example: Gifford's Read/Write Quoroms

- Object (e.g. file) read/write operations are subject to two constraints:
  1. Each final for write must intersect the initial each initial quorom for read.
  2. Each final quorom for read must intersect the initial quorom for write.
     - To ensure that version are updated properly.

- Choices of minimal (size) quorom for an object with 5 replicas

<table>
    <tr>
        <th> Operation </td>
        <th colspan="3" text-align="center"> Quorom Choices </td>
    </tr>
    <tr>
        <td> Read </td>
        <td> (1,0) </td>
        <td> (2,0) </td>
        <td> (3,0) </td>
    </tr>
    <tr>
        <td> Write </td>
        <td> (1,5) </td>
        <td> (2,4) </td>
        <td> (3,3) </td>
    </tr>
</table>

