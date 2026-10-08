// ============================================================
//  CleanCycle – Backend Server
//  Express.js | REST API | JSON file storage
//  Run: node server.js  →  http://localhost:3000
// ============================================================

const express = require('express');
const cors    = require('cors');
const path    = require('path');
const fs      = require('fs');

const app  = express();
const PORT = 3000;
const DB_FILE = path.join(__dirname, 'data.json');

// ── Middleware ────────────────────────────────────────────
app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

// ── Init DB file if missing ───────────────────────────────
function initDB() {
  if (!fs.existsSync(DB_FILE)) {
    const initial = {
      entries: [],
      totalCount: 0,
      totalWeightGrams: 0,
      createdAt: new Date().toISOString()
    };
    fs.writeFileSync(DB_FILE, JSON.stringify(initial, null, 2));
    console.log('[DB] data.json created.');
  }
}

function readDB() {
  return JSON.parse(fs.readFileSync(DB_FILE, 'utf8'));
}

function writeDB(data) {
  fs.writeFileSync(DB_FILE, JSON.stringify(data, null, 2));
}

initDB();

// ── ROUTES ────────────────────────────────────────────────

// GET /api/stats  → full stats + entries
app.get('/api/stats', (req, res) => {
  const db = readDB();
  res.json({
    success: true,
    totalCount:       db.totalCount,
    totalWeightGrams: db.totalWeightGrams,
    entries:          db.entries.slice().reverse(), // newest first
    lastEntry:        db.entries[db.entries.length - 1] || null
  });
});

// POST /api/collect  → add a collection entry
// Body: { material, weightGrams, location, notes, source }
app.post('/api/collect', (req, res) => {
  const { material = 'PET', weightGrams = 25, location = 'Bin A', notes = '', source = 'manual' } = req.body;

  if (!['PET', 'HDPE', 'PP', 'LDPE', 'OTHER'].includes(material)) {
    return res.status(400).json({ success: false, error: 'Invalid material type.' });
  }

  const db = readDB();
  const entry = {
    id:           db.totalCount + 1,
    timestamp:    new Date().toISOString(),
    material,
    weightGrams:  Number(weightGrams),
    location,
    notes,
    source        // 'manual' | 'arduino' | 'simulated'
  };

  db.entries.push(entry);
  db.totalCount       += 1;
  db.totalWeightGrams += Number(weightGrams);

  writeDB(db);

  console.log(`[COLLECT] #${entry.id} | ${material} | ${weightGrams}g | ${source}`);

  res.json({ success: true, entry, totalCount: db.totalCount, totalWeightGrams: db.totalWeightGrams });
});

// DELETE /api/reset  → wipe all data
app.delete('/api/reset', (req, res) => {
  writeDB({ entries: [], totalCount: 0, totalWeightGrams: 0, createdAt: new Date().toISOString() });
  console.log('[RESET] All data wiped.');
  res.json({ success: true, message: 'Data reset.' });
});

// DELETE /api/entry/:id  → delete single entry
app.delete('/api/entry/:id', (req, res) => {
  const id = parseInt(req.params.id);
  const db = readDB();
  const idx = db.entries.findIndex(e => e.id === id);
  if (idx === -1) return res.status(404).json({ success: false, error: 'Entry not found.' });

  const removed = db.entries.splice(idx, 1)[0];
  db.totalCount       = Math.max(0, db.totalCount - 1);
  db.totalWeightGrams = Math.max(0, db.totalWeightGrams - removed.weightGrams);

  writeDB(db);
  res.json({ success: true, removed });
});

// ── Serve frontend ────────────────────────────────────────
app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// ── Start ─────────────────────────────────────────────────
app.listen(PORT, () => {
  console.log('');
  console.log('╔══════════════════════════════════════════════╗');
  console.log('║   ♻️  CleanCycle Backend  –  RUNNING          ║');
  console.log(`║   http://localhost:${PORT}                     ║`);
  console.log('║   Press Ctrl+C to stop                       ║');
  console.log('╚══════════════════════════════════════════════╝');
  console.log('');
});
