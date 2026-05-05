import { useState } from "react";
import { LineChart, Line, BarChart, Bar, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from "recharts";

const SIZES = [1000, 5000, 10000, 50000, 100000];
const SIZE_LABELS = ["1K", "5K", "10K", "50K", "100K"];

const BST_SCHEMA = [
  ["time", "f"],
  ["rotations", "i"],
  ["comparisons", "i"],
  ["height", "i"],
];

const BT_SCHEMA = [
  ["time", "f"],
  ["splits", "i"],
  ["merges", "i"],
  ["comparisons", "i"],
  ["node_accesses", "i"],
  ["height", "i"],
  ["fill_factor", "f"],
];

function parseCSV(text, schema) {
  const lines = text.trim().split(/\r?\n/).filter(Boolean);
  const header = lines[0].split(",").map((h) => h.trim());

  return lines
    .slice(1)
    .map((line) => {
      const vals = line.split(",").map((v) => v.trim());
      const obj = {};

      header.forEach((h, i) => {
        obj[h] = vals[i] || "0";
      });

      const row = {
        tree: obj.tree,
        operation: obj.operation,
        pattern: obj.pattern,
        n: Number(obj.n),
      };

      schema.forEach(([field, type]) => {
        row[field] =
          type === "f"
            ? parseFloat(obj[field] || 0)
            : parseInt(obj[field] || 0, 10);
      });

      return row;
    })
    .filter((r) => r.tree && r.operation && r.n);
}

// ─── Design Tokens ────────────────────────────────────────────────────────────
const C = {
  avl: "#E84A4A", avlFill: "rgba(232,74,74,0.15)",
  rb:  "#3B82F6", rbFill:  "rgba(59,130,246,0.15)",
  splay: "#22C55E", splayFill: "rgba(34,197,94,0.15)",
  btree: "#A855F7", btreeFill: "rgba(168,85,247,0.15)",
  bplus: "#F59E0B", bplusFill: "rgba(245,158,11,0.15)",
  log: "#94A3B8",
};

function get(data, tree, op, pat, n, key) {
  if (!data) return 0;
  const r = data.find(r => r.tree === tree && r.operation === op && r.pattern === pat && +r.n === n);
  return r ? (r[key] ?? 0) : 0;
}

// ─── Recharts Custom Tooltip ──────────────────────────────────────────────────
const CustomTooltip = ({ active, payload, label }) => {
  if (!active || !payload?.length) return null;
  return (
    <div style={{ background: "#0F172A", border: "1px solid #1E293B", borderRadius: 8, padding: "10px 14px", fontSize: 12, fontFamily: "monospace" }}>
      <div style={{ color: "#94A3B8", marginBottom: 6, fontWeight: 600 }}>n = {label}</div>
      {payload.map((p, i) => (
        <div key={i} style={{ color: p.color, marginBottom: 2 }}>
          {p.name}: <span style={{ color: "#F1F5F9", fontWeight: 700 }}>{typeof p.value === "number" ? p.value.toLocaleString() : p.value}</span>
        </div>
      ))}
    </div>
  );
};

// ─── Pill / Tab components ────────────────────────────────────────────────────
function Tabs({ options, value, onChange, small }) {
  return (
    <div style={{ display: "flex", gap: 2, flexWrap: "wrap" }}>
      {options.map(o => (
        <button key={o.value} onClick={() => onChange(o.value)}
          style={{
            padding: small ? "4px 10px" : "6px 14px",
            fontSize: small ? 11 : 12, fontFamily: "monospace", fontWeight: 600,
            border: "1.5px solid", borderRadius: 6, cursor: "pointer", transition: "all 0.15s",
            borderColor: value === o.value ? "#6366F1" : "#1E293B",
            background: value === o.value ? "#6366F1" : "#0F172A",
            color: value === o.value ? "#fff" : "#94A3B8",
          }}>
          {o.label}
        </button>
      ))}
    </div>
  );
}

// ─── Card ─────────────────────────────────────────────────────────────────────
function Card({ title, subtitle, children, full }) {
  return (
    <div style={{
      gridColumn: full ? "1 / -1" : undefined,
      background: "#0F172A", border: "1px solid #1E293B", borderRadius: 12,
      padding: "20px 22px", display: "flex", flexDirection: "column", gap: 16,
    }}>
      <div>
        <div style={{ fontFamily: "'Space Grotesk', monospace", fontSize: 13, fontWeight: 700, color: "#F1F5F9", letterSpacing: "0.04em", textTransform: "uppercase" }}>{title}</div>
        {subtitle && <div style={{ fontSize: 11, color: "#475569", marginTop: 3, fontFamily: "monospace" }}>{subtitle}</div>}
      </div>
      {children}
    </div>
  );
}

// ─── Stat Mini Card ───────────────────────────────────────────────────────────
function StatCard({ label, entries, winner }) {
  return (
    <div style={{ background: "#070F1E", border: "1px solid #1E293B", borderRadius: 8, padding: "12px 14px" }}>
      <div style={{ fontSize: 10, color: "#475569", fontFamily: "monospace", textTransform: "uppercase", letterSpacing: "0.1em", marginBottom: 10 }}>{label}</div>
      {entries.map((e, i) => {
        const isWinner = e.val === winner && e.val > 0;
        return (
          <div key={i} style={{
            display: "flex", justifyContent: "space-between", alignItems: "center",
            padding: "5px 8px", borderRadius: 5, marginBottom: 4,
            background: `${e.color}12`,
          }}>
            <span style={{ color: e.color, fontSize: 11, fontWeight: 700, fontFamily: "monospace" }}>{e.label}</span>
            <span style={{ fontSize: 11, fontFamily: "monospace", fontWeight: isWinner ? 700 : 400, color: isWinner ? "#22C55E" : "#94A3B8" }}>
              {isWinner && "★ "}{e.val.toLocaleString()}
            </span>
          </div>
        );
      })}
    </div>
  );
}

// ─── BST Section ──────────────────────────────────────────────────────────────
function BSTSection({ data, pattern }) {
  const [metric, setMetric] = useState("rotations");
  const [op, setOp] = useState("insert");
  const [rotSize, setRotSize] = useState(100000);

  const metricLine = SIZES.map((n, i) => ({
    n: SIZE_LABELS[i],
    AVL: get(data, "AVL", op, pattern, n, metric),
    "Red-Black": get(data, "RB", op, pattern, n, metric),
    Splay: get(data, "Splay", op, pattern, n, metric),
  }));

  const heightLine = SIZES.map((n, i) => ({
    n: SIZE_LABELS[i],
    AVL: get(data, "AVL", "insert", pattern, n, "height"),
    "Red-Black": get(data, "RB", "insert", pattern, n, "height"),
    Splay: get(data, "Splay", "insert", pattern, n, "height"),
    "log₂(n)": Math.round(Math.log2(n)),
  }));

  const timeLine = SIZES.map((n, i) => ({
    n: SIZE_LABELS[i],
    AVL: get(data, "AVL", op, pattern, n, "time"),
    "Red-Black": get(data, "RB", op, pattern, n, "time"),
    Splay: get(data, "Splay", op, pattern, n, "time"),
  }));
  
  const rotBar = ["Insert", "Delete"].map(o => ({
    op: o,
    AVL: get(data, "AVL", o.toLowerCase(), pattern, rotSize, "rotations"),
    "Red-Black": get(data, "RB", o.toLowerCase(), pattern, rotSize, "rotations"),
    Splay: get(data, "Splay", o.toLowerCase(), pattern, rotSize, "rotations"),
  }));

  const statDefs = [
    { key: "rotations", label: "Insert Rotations (n=100K)", op: "insert" },
    { key: "comparisons", label: "Search Comparisons (n=100K)", op: "search" },
    { key: "height", label: "Tree Height after Insert (n=100K)", op: "insert" },
  ];

  return (
    <div style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: 16 }}>

      {/* Summary stats */}
      <Card title="Summary — n = 100,000" subtitle={`${pattern} input pattern · ★ = best (lowest)`} full>
        <div style={{ display: "grid", gridTemplateColumns: "repeat(3,1fr)", gap: 12 }}>
          {statDefs.map(s => {
            const entries = [
              { label: "AVL", color: C.avl, val: get(data, "AVL", s.op, pattern, 100000, s.key) },
              { label: "Red-Black", color: C.rb, val: get(data, "RB", s.op, pattern, 100000, s.key) },
              { label: "Splay", color: C.splay, val: get(data, "Splay", s.op, pattern, 100000, s.key) },
            ];
            const winner = Math.min(...entries.filter(e => e.val > 0).map(e => e.val));
            return <StatCard key={s.key} label={s.label} entries={entries} winner={winner} />;
          })}
        </div>
      </Card>

      {/* Metric line chart */}
      <Card
        title={`${metric.charAt(0).toUpperCase() + metric.slice(1)} — ${op} operation`}
        subtitle="across all input sizes"
        full
      >
        <div style={{ display: "flex", gap: 12, flexWrap: "wrap", marginBottom: 4 }}>
          <Tabs small options={[{ value: "insert", label: "Insert" }, { value: "search", label: "Search" }, { value: "delete", label: "Delete" }]} value={op} onChange={setOp} />
          <Tabs small options={[{ value: "rotations", label: "Rotations" }, { value: "comparisons", label: "Comparisons" }, { value: "height", label: "Height" }, { value: "time", label: "Time (s)" }]} value={metric} onChange={setMetric} />
        </div>
        <ResponsiveContainer width="100%" height={260}>
          <LineChart data={metric === "time" ? timeLine : metricLine} margin={{ top: 4, right: 16, left: 0, bottom: 0 }}>
            <CartesianGrid strokeDasharray="3 3" stroke="#1E293B" />
            <XAxis dataKey="n" tick={{ fill: "#475569", fontSize: 11, fontFamily: "monospace" }} />
            <YAxis tick={{ fill: "#475569", fontSize: 10, fontFamily: "monospace" }} tickFormatter={v => v >= 1e6 ? (v/1e6).toFixed(1)+"M" : v >= 1e3 ? (v/1e3).toFixed(0)+"K" : v} width={60} />
            <Tooltip content={<CustomTooltip />} />
            <Legend wrapperStyle={{ fontSize: 11, fontFamily: "monospace" }} />
            <Line type="monotone" dataKey="AVL" stroke={C.avl} strokeWidth={2.5} dot={{ r: 4, fill: C.avl }} activeDot={{ r: 6 }} />
            <Line type="monotone" dataKey="Red-Black" stroke={C.rb} strokeWidth={2.5} dot={{ r: 4, fill: C.rb }} activeDot={{ r: 6 }} />
            <Line type="monotone" dataKey="Splay" stroke={C.splay} strokeWidth={2.5} dot={{ r: 4, fill: C.splay }} activeDot={{ r: 6 }} />
          </LineChart>
        </ResponsiveContainer>
      </Card>

      {/* Height vs log2(n) */}
      <Card title="Tree Height after Insert" subtitle="vs theoretical log₂(n) bound">
        <ResponsiveContainer width="100%" height={230}>
          <LineChart data={heightLine} margin={{ top: 4, right: 16, left: 0, bottom: 0 }}>
            <CartesianGrid strokeDasharray="3 3" stroke="#1E293B" />
            <XAxis dataKey="n" tick={{ fill: "#475569", fontSize: 11, fontFamily: "monospace" }} />
            <YAxis tick={{ fill: "#475569", fontSize: 10, fontFamily: "monospace" }} width={45} />
            <Tooltip content={<CustomTooltip />} />
            <Legend wrapperStyle={{ fontSize: 11, fontFamily: "monospace" }} />
            <Line type="monotone" dataKey="AVL" stroke={C.avl} strokeWidth={2.5} dot={{ r: 4, fill: C.avl }} />
            <Line type="monotone" dataKey="Red-Black" stroke={C.rb} strokeWidth={2.5} dot={{ r: 4, fill: C.rb }} />
            <Line type="monotone" dataKey="Splay" stroke={C.splay} strokeWidth={2.5} dot={{ r: 4, fill: C.splay }} />
            <Line type="monotone" dataKey="log₂(n)" stroke={C.log} strokeWidth={1.5} strokeDasharray="6 3" dot={false} />
          </LineChart>
        </ResponsiveContainer>
      </Card>

      {/* Rotations bar */}
      <Card title={`Rotations at n = ${rotSize.toLocaleString()}`} subtitle="insert & delete operations">
        <div style={{ display: "flex", gap: 12, flexWrap: "wrap", marginBottom: 4 }}>
          <Tabs small options={SIZES.map((s, i) => ({ value: s, label: SIZE_LABELS[i] }))} value={rotSize} onChange={setRotSize} />
        </div>
        <ResponsiveContainer width="100%" height={230}>
          <BarChart data={rotBar} margin={{ top: 4, right: 16, left: 0, bottom: 0 }}>
            <CartesianGrid strokeDasharray="3 3" stroke="#1E293B" />
            <XAxis dataKey="op" tick={{ fill: "#475569", fontSize: 11, fontFamily: "monospace" }} />
            <YAxis tick={{ fill: "#475569", fontSize: 10, fontFamily: "monospace" }} tickFormatter={v => v >= 1e3 ? (v/1e3).toFixed(0)+"K" : v} width={55} />
            <Tooltip content={<CustomTooltip />} />
            <Legend wrapperStyle={{ fontSize: 11, fontFamily: "monospace" }} />
            <Bar dataKey="AVL" fill={C.avlFill} stroke={C.avl} strokeWidth={1.5} radius={[4,4,0,0]} />
            <Bar dataKey="Red-Black" fill={C.rbFill} stroke={C.rb} strokeWidth={1.5} radius={[4,4,0,0]} />
            <Bar dataKey="Splay" fill={C.splayFill} stroke={C.splay} strokeWidth={1.5} radius={[4,4,0,0]} />
          </BarChart>
        </ResponsiveContainer>
      </Card>

      {/* Full data table */}
      <Card title="Full Data Table" subtitle={`${pattern} pattern · all operations & sizes`} full>
        <div style={{ overflowX: "auto" }}>
          <table style={{ width: "100%", borderCollapse: "collapse", fontSize: 11, fontFamily: "monospace" }}>
            <thead>
              <tr style={{ borderBottom: "1px solid #1E293B" }}>
                {["Tree","Op","n","Rotations","Comparisons","Height","Time (s)"].map(h => (
                  <th key={h} style={{ padding: "6px 10px", color: "#475569", textAlign: "left", fontWeight: 600, textTransform: "uppercase", fontSize: 10, letterSpacing: "0.08em" }}>{h}</th>
                ))}
              </tr>
            </thead>
            <tbody>
              {data.filter(r => r.pattern === pattern).map((r, i) => (
                <tr key={i} style={{ borderBottom: "1px solid #0F172A" }}>
                  <td style={{ padding: "5px 10px", color: r.tree === "AVL" ? C.avl : r.tree === "RB" ? C.rb : C.splay, fontWeight: 700 }}>{r.tree === "RB" ? "Red-Black" : r.tree}</td>
                  <td style={{ padding: "5px 10px", color: "#94A3B8" }}>{r.operation}</td>
                  <td style={{ padding: "5px 10px", color: "#94A3B8" }}>{r.n.toLocaleString()}</td>
                  <td style={{ padding: "5px 10px", color: "#CBD5E1" }}>{r.rotations.toLocaleString()}</td>
                  <td style={{ padding: "5px 10px", color: "#CBD5E1" }}>{r.comparisons.toLocaleString()}</td>
                  <td style={{ padding: "5px 10px", color: "#CBD5E1" }}>{r.height}</td>
                  <td style={{ padding: "5px 10px", color: "#CBD5E1" }}>{r.time.toFixed(4)}</td>
                </tr>
              ))}
              {data.length === 0 && <tr><td colSpan="7" style={{ padding: "20px", textAlign: "center", color: "#475569" }}>No data available. Upload slightly more data csv.</td></tr>}
            </tbody>
          </table>
        </div>
      </Card>
    </div>
  );
}

// ─── B-Tree Section ───────────────────────────────────────────────────────────
function BTreeSection({ data, pattern }) {
  const [metric, setMetric] = useState("splits");
  const [op, setOp] = useState("insert");

  // const metricLine = SIZES.map((n, i) => {
  //   const row = { n: SIZE_LABELS[i] };
  //   row["B-Tree"] = get(data, "BTree", op, pattern, n, metric);
  //   if (op !== "delete") row["B+ Tree"] = get(data, "BPlusTree", op, pattern, n, metric);
  //   return row;
  // });
  // FIXED — always include B+ Tree
  const metricLine = SIZES.map((n, i) => ({
    n: SIZE_LABELS[i],
    "B-Tree":  get(data, "BTree",     op, pattern, n, metric),
    "B+ Tree": get(data, "BPlusTree", op, pattern, n, metric),
  }));


  const heightLine = SIZES.map((n, i) => ({
    n: SIZE_LABELS[i],
    "B-Tree": get(data, "BTree", "insert", pattern, n, "height"),
    "B+ Tree": get(data, "BPlusTree", "insert", pattern, n, "height"),
    "log₅(n)": +(Math.log(n) / Math.log(5)).toFixed(2),
  }));

  const fillLine = SIZES.map((n, i) => ({
    n: SIZE_LABELS[i],
    "B-Tree": get(data, "BTree", "insert", pattern, n, "fill_factor"),
    "B+ Tree": get(data, "BPlusTree", "insert", pattern, n, "fill_factor"),
  }));

  const rangeLine = SIZES.map((n, i) => ({
    n: SIZE_LABELS[i],
    "Leaf Accesses": get(data, "BPlusTree", "range_query", pattern, n, "node_accesses"),
    "Comparisons": get(data, "BPlusTree", "range_query", pattern, n, "comparisons"),
  }));

  const splitBar = SIZES.map((n, i) => ({
    n: SIZE_LABELS[i],
    "B-Tree": get(data, "BTree", "insert", pattern, n, "splits"),
    "B+ Tree": get(data, "BPlusTree", "insert", pattern, n, "splits"),
  }));

  const statDefs = [
    { key: "splits", label: "Insert Splits (n=100K)", op: "insert" },
    { key: "node_accesses", label: "Search Node Accesses (n=100K)", op: "search" },
    { key: "height", label: "Height after Insert (n=100K)", op: "insert" },
    { key: "fill_factor", label: "Fill Factor % (n=100K)", op: "insert" },
  ];

  const metricLabel = { splits: "Splits", merges: "Merges", comparisons: "Comparisons", node_accesses: "Node Accesses", height: "Height", fill_factor: "Fill Factor %" };

  return (
    <div style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: 16 }}>

      {/* Summary */}
      <Card title="Summary — n = 100,000" subtitle={`${pattern} input pattern · ★ = best (lowest for splits/accesses/height, highest for fill%)`} full>
        <div style={{ display: "grid", gridTemplateColumns: "repeat(4,1fr)", gap: 12 }}>
          {statDefs.map(s => {
            const entries = [
              { label: "B-Tree ", color: C.btree, val: get(data, "BTree", s.op, pattern, 100000, s.key) },
              { label: "B+ Tree ", color: C.bplus, val: get(data, "BPlusTree", s.op, pattern, 100000, s.key) },
            ];
            const winner = s.key === "fill_factor"
              ? Math.max(...entries.map(e => e.val))
              : Math.min(...entries.filter(e => e.val > 0).map(e => e.val));
            return <StatCard key={s.key} label={s.label} entries={entries} winner={winner} />;
          })}
        </div>
      </Card>

      {/* Metric line */}
      <Card
        title={`${metricLabel[metric] || metric} — ${op} operation`}
        subtitle="across all input sizes"
        full
      >
        <div style={{ display: "flex", gap: 12, flexWrap: "wrap", marginBottom: 4 }}>
          <Tabs small options={[
            { value: "insert", label: "Insert" },
            { value: "search", label: "Search" },
            { value: "delete", label: "Delete " },
          ]} value={op} onChange={setOp} />
          <Tabs small options={[
            { value: "splits", label: "Splits" },
            { value: "comparisons", label: "Comparisons" },
            { value: "node_accesses", label: "Node Accesses" },
            { value: "height", label: "Height" },
            { value: "fill_factor", label: "Fill Factor" },
            { value: "time", label: "Time (s)" },
          ]} value={metric} onChange={setMetric} />
        </div>
        <ResponsiveContainer width="100%" height={260}>
          <LineChart data={metricLine} margin={{ top: 4, right: 16, left: 0, bottom: 0 }}>
            <CartesianGrid strokeDasharray="3 3" stroke="#1E293B" />
            <XAxis dataKey="n" tick={{ fill: "#475569", fontSize: 11, fontFamily: "monospace" }} />
            <YAxis tick={{ fill: "#475569", fontSize: 10, fontFamily: "monospace" }} tickFormatter={v => v >= 1e6 ? (v/1e6).toFixed(1)+"M" : v >= 1e3 ? (v/1e3).toFixed(0)+"K" : v} width={65} />
            <Tooltip content={<CustomTooltip />} />
            <Legend wrapperStyle={{ fontSize: 11, fontFamily: "monospace" }} />
            <Line type="monotone" dataKey="B-Tree" stroke={C.btree} strokeWidth={2.5} dot={{ r: 4, fill: C.btree }} activeDot={{ r: 6 }} />
            {/* {op !== "delete" && <Line type="monotone" dataKey="B+ Tree" stroke={C.bplus} strokeWidth={2.5} dot={{ r: 4, fill: C.bplus }} activeDot={{ r: 6 }} />} */}
            <Line type="monotone" dataKey="B+ Tree" stroke={C.bplus} strokeWidth={2.5} dot={{ r: 4, fill: C.bplus }} activeDot={{ r: 6 }} />
          </LineChart>
        </ResponsiveContainer>
      </Card>

      {/* Height */}
      <Card title="Tree Height after Insert" subtitle="vs log₅(n) theoretical bound (order 5)">
        <ResponsiveContainer width="100%" height={230}>
          <LineChart data={heightLine} margin={{ top: 4, right: 16, left: 0, bottom: 0 }}>
            <CartesianGrid strokeDasharray="3 3" stroke="#1E293B" />
            <XAxis dataKey="n" tick={{ fill: "#475569", fontSize: 11, fontFamily: "monospace" }} />
            <YAxis tick={{ fill: "#475569", fontSize: 10, fontFamily: "monospace" }} width={40} />
            <Tooltip content={<CustomTooltip />} />
            <Legend wrapperStyle={{ fontSize: 11, fontFamily: "monospace" }} />
            <Line type="monotone" dataKey="B-Tree" stroke={C.btree} strokeWidth={2.5} dot={{ r: 4, fill: C.btree }} />
            <Line type="monotone" dataKey="B+ Tree" stroke={C.bplus} strokeWidth={2.5} dot={{ r: 4, fill: C.bplus }} />
            <Line type="monotone" dataKey="log₅(n)" stroke={C.log} strokeWidth={1.5} strokeDasharray="6 3" dot={false} />
          </LineChart>
        </ResponsiveContainer>
      </Card>

      {/* Fill Factor */}
      <Card title="Fill Factor %" subtitle="node utilization after insert (higher = better space usage)">
        <ResponsiveContainer width="100%" height={230}>
          <LineChart data={fillLine} margin={{ top: 4, right: 16, left: 0, bottom: 0 }}>
            <CartesianGrid strokeDasharray="3 3" stroke="#1E293B" />
            <XAxis dataKey="n" tick={{ fill: "#475569", fontSize: 11, fontFamily: "monospace" }} />
            <YAxis domain={[0, 100]} tick={{ fill: "#475569", fontSize: 10, fontFamily: "monospace" }} tickFormatter={v => v + "%"} width={45} />
            <Tooltip content={<CustomTooltip />} />
            <Legend wrapperStyle={{ fontSize: 11, fontFamily: "monospace" }} />
            <Line type="monotone" dataKey="B-Tree" stroke={C.btree} strokeWidth={2.5} dot={{ r: 4, fill: C.btree }} />
            <Line type="monotone" dataKey="B+ Tree" stroke={C.bplus} strokeWidth={2.5} dot={{ r: 4, fill: C.bplus }} />
          </LineChart>
        </ResponsiveContainer>
      </Card>

      {/* Splits bar chart */}
      <Card title="Splits during Insert" subtitle="by input size and tree type">
        <ResponsiveContainer width="100%" height={230}>
          <BarChart data={splitBar} margin={{ top: 4, right: 16, left: 0, bottom: 0 }}>
            <CartesianGrid strokeDasharray="3 3" stroke="#1E293B" />
            <XAxis dataKey="n" tick={{ fill: "#475569", fontSize: 11, fontFamily: "monospace" }} />
            <YAxis tick={{ fill: "#475569", fontSize: 10, fontFamily: "monospace" }} tickFormatter={v => v >= 1e3 ? (v/1e3).toFixed(0)+"K" : v} width={55} />
            <Tooltip content={<CustomTooltip />} />
            <Legend wrapperStyle={{ fontSize: 11, fontFamily: "monospace" }} />
            <Bar dataKey="B-Tree" fill={C.btreeFill} stroke={C.btree} strokeWidth={1.5} radius={[4,4,0,0]} />
            <Bar dataKey="B+ Tree" fill={C.bplusFill} stroke={C.bplus} strokeWidth={1.5} radius={[4,4,0,0]} />
          </BarChart>
        </ResponsiveContainer>
      </Card>

      {/* B+ Range Query */}
      <Card title="B+ Tree — Range Query" subtitle="10% window around median · leaf accesses & comparisons">
        <ResponsiveContainer width="100%" height={230}>
          <LineChart data={rangeLine} margin={{ top: 4, right: 16, left: 0, bottom: 0 }}>
            <CartesianGrid strokeDasharray="3 3" stroke="#1E293B" />
            <XAxis dataKey="n" tick={{ fill: "#475569", fontSize: 11, fontFamily: "monospace" }} />
            <YAxis tick={{ fill: "#475569", fontSize: 10, fontFamily: "monospace" }} tickFormatter={v => v >= 1e3 ? (v/1e3).toFixed(0)+"K" : v} width={55} />
            <Tooltip content={<CustomTooltip />} />
            <Legend wrapperStyle={{ fontSize: 11, fontFamily: "monospace" }} />
            <Line type="monotone" dataKey="Leaf Accesses" stroke={C.bplus} strokeWidth={2.5} dot={{ r: 4, fill: C.bplus }} />
            <Line type="monotone" dataKey="Comparisons" stroke="#38BDF8" strokeWidth={2} strokeDasharray="5 3" dot={{ r: 3, fill: "#38BDF8" }} />
          </LineChart>
        </ResponsiveContainer>
      </Card>

      {/* Table */}
      <Card title="Full Data Table" subtitle={`${pattern} pattern · all operations & sizes`} full>
        <div style={{ overflowX: "auto" }}>
          <table style={{ width: "100%", borderCollapse: "collapse", fontSize: 11, fontFamily: "monospace" }}>
            <thead>
              <tr style={{ borderBottom: "1px solid #1E293B" }}>
                {["Tree","Op","n","Splits","Merges","Comparisons","Node Acc.","Height","Fill %","Time (s)"].map(h => (
                  <th key={h} style={{ padding: "6px 10px", color: "#475569", textAlign: "left", fontWeight: 600, textTransform: "uppercase", fontSize: 10, letterSpacing: "0.08em" }}>{h}</th>
                ))}
              </tr>
            </thead>
            <tbody>
              {data.filter(r => r.pattern === pattern).map((r, i) => (
                <tr key={i} style={{ borderBottom: "1px solid #0F172A" }}>
                  <td style={{ padding: "5px 10px", color: r.tree === "BTree" ? C.btree : C.bplus, fontWeight: 700 }}>{r.tree === "BTree" ? "B-Tree" : "B+ Tree"}</td>
                  <td style={{ padding: "5px 10px", color: "#94A3B8" }}>{r.operation}</td>
                  <td style={{ padding: "5px 10px", color: "#94A3B8" }}>{r.n.toLocaleString()}</td>
                  <td style={{ padding: "5px 10px", color: "#CBD5E1" }}>{r.splits.toLocaleString()}</td>
                  <td style={{ padding: "5px 10px", color: "#CBD5E1" }}>{r.merges.toLocaleString()}</td>
                  <td style={{ padding: "5px 10px", color: "#CBD5E1" }}>{r.comparisons.toLocaleString()}</td>
                  <td style={{ padding: "5px 10px", color: "#CBD5E1" }}>{r.node_accesses.toLocaleString()}</td>
                  <td style={{ padding: "5px 10px", color: "#CBD5E1" }}>{r.height}</td>
                  <td style={{ padding: "5px 10px", color: "#CBD5E1" }}>{r.fill_factor.toFixed(1)}%</td>
                  <td style={{ padding: "5px 10px", color: "#CBD5E1" }}>{r.time.toFixed(4)}</td>
                </tr>
              ))}
              {data.length === 0 && <tr><td colSpan="10" style={{ padding: "20px", textAlign: "center", color: "#475569" }}>No data available. Upload tree data csv.</td></tr>}
            </tbody>
          </table>
        </div>
      </Card>
    </div>
  );
}

// ─── Root App ─────────────────────────────────────────────────────────────────
export default function Dashboard() {
  const [section, setSection] = useState("bst");
  const [pattern, setPattern] = useState("random");
  const [bstRows, setBstRows] = useState([]);
  const [btRows, setBtRows] = useState([]);
  const [status, setStatus] = useState("Select results.csv to populate charts");

  async function handleUpload(e) {
    const file = e.target.files?.[0];
    if (!file) return;

    const text = await file.text();

    if (text.startsWith("tree,operation,pattern,n,time,rotations")) {
      const rows = parseCSV(text, BST_SCHEMA);
      setBstRows(rows);
      setSection("bst");
      setStatus(`✓ BST loaded: ${rows.length} rows`);
      return;
    }

    if (text.startsWith("tree,operation,pattern,n,time,splits")) {
      const rows = parseCSV(text, BT_SCHEMA);
      setBtRows(rows);
      setSection("btree");
      setStatus(`✓ B-Trees loaded: ${rows.length} rows`);
      return;
    }

    setStatus("✗ Unknown CSV format");
  }

  const sectionOpts = [{ value: "bst", label: "BST Family" }, { value: "btree", label: "B-Tree Family" }];
  const patternOpts = [{ value: "random", label: "Random" }, { value: "sorted", label: "Sorted" }, { value: "reverse", label: "Reverse" }];

  return (
    <div style={{ minHeight: "100vh", background: "#020817", color: "#F1F5F9", fontFamily: "monospace" }}>
      {/* Header */}
      <div style={{
        background: "#0A1628", borderBottom: "1px solid #1E293B",
        padding: "18px 28px", display: "flex", alignItems: "center", justifyContent: "space-between", flexWrap: "wrap", gap: 12
      }}>
        <div>
          <div style={{ fontFamily: "'Space Grotesk', monospace", fontSize: 22, fontWeight: 800, letterSpacing: "0.02em", color: "#F1F5F9" }}>
            Algorithm Performance
            <span style={{ color: "#6366F1", marginLeft: 8 }}>Dashboard</span>
          </div>
          <div style={{ fontSize: 11, color: "#475569", marginTop: 3, letterSpacing: "0.08em" }}>
            {section === "bst" ? "AVL · Red-Black · Splay Tree" : "B-Tree · B+ Tree "}
          </div>
        </div>
        <div style={{ display: "flex", alignItems: "center", gap: 8, flexWrap: "wrap" }}>
          {section === "bst" ? (
            <>
              {[{ label: "AVL Tree", color: C.avl }, { label: "Red-Black", color: C.rb }, { label: "Splay Tree", color: C.splay }].map(l => (
                <div key={l.label} style={{ display: "flex", alignItems: "center", gap: 6, fontSize: 11, fontWeight: 700 }}>
                  <div style={{ width: 10, height: 10, borderRadius: 2, background: l.color }} />
                  <span style={{ color: l.color }}>{l.label}</span>
                </div>
              ))}
            </>
          ) : (
            <>
              {[{ label: "B-Tree", color: C.btree }, { label: "B+ Tree", color: C.bplus }].map(l => (
                <div key={l.label} style={{ display: "flex", alignItems: "center", gap: 6, fontSize: 11, fontWeight: 700 }}>
                  <div style={{ width: 10, height: 10, borderRadius: 2, background: l.color }} />
                  <span style={{ color: l.color }}>{l.label}</span>
                </div>
              ))}
            </>
          )}
        </div>
      </div>

      {/* Controls */}
      <div style={{
        background: "#070F1E", borderBottom: "1px solid #1E293B",
        padding: "12px 28px", display: "flex", alignItems: "center", gap: 20, flexWrap: "wrap", justifyContent: "space-between"
      }}>
        <div style={{ display: "flex", alignItems: "center", gap: 20, flexWrap: "wrap" }}>
          <Tabs options={sectionOpts} value={section} onChange={setSection} />
          <div style={{ width: 1, height: 28, background: "#1E293B" }} />
          <div style={{ display: "flex", alignItems: "center", gap: 8 }}>
            <span style={{ fontSize: 10, color: "#475569", textTransform: "uppercase", letterSpacing: "0.12em", fontWeight: 600 }}>Pattern</span>
            <Tabs options={patternOpts} value={pattern} onChange={setPattern} />
          </div>
        </div>
        
        <div style={{ display: "flex", alignItems: "center", gap: 12 }}>
          <span style={{ fontSize: 12, color: "#94A3B8" }}>{status}</span>
          <label style={{
             background: "#6366F1", color: "#fff", padding: "6px 14px", borderRadius: 6,
             fontSize: 12, fontWeight: 700, cursor: "pointer", transition: "all 0.15s"
          }}>
            Upload CSV
            <input type="file" accept=".csv" onChange={handleUpload} style={{ display: "none" }} />
          </label>
        </div>
      </div>

      {/* Content */}
      <div style={{ padding: "20px 28px 40px" }}>
        {section === "bst" ? <BSTSection data={bstRows} pattern={pattern} /> : <BTreeSection data={btRows} pattern={pattern} />}
      </div>
    </div>
  );
}