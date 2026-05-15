// PenguinBot WebUI - app.js
const WS_URL = `ws://${location.hostname}:81/ws`;
let ws, telemetryHistory = [], histMax = 200;

// ===== WebSocket =====
function connectWS() {
  ws = new WebSocket(WS_URL);
  ws.onopen = () => {
    document.getElementById('ws-status').className = 'dot online';
    document.getElementById('ws-status').title = '已连接';
    send({cmd:'pid_get'});
  };
  ws.onclose = () => {
    document.getElementById('ws-status').className = 'dot offline';
    document.getElementById('ws-status').title = '断开, 5s重连';
    setTimeout(connectWS, 5000);
  };
  ws.onerror = () => ws.close();
  ws.onmessage = (e) => {
    const msg = JSON.parse(e.data);
    if (msg.type === 'telemetry') handleTelemetry(msg.data);
    else if (msg.type === 'event') console.log('Event:', msg);
  };
}

function send(obj) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify(obj));
  }
}

// ===== Telemetry =====
function handleTelemetry(d) {
  document.getElementById('t-angle').textContent = d.angle.toFixed(2) + '°';
  document.getElementById('t-gyro').textContent  = d.angular_velocity.toFixed(2) + '°/s';
  document.getElementById('t-lpwm').textContent  = d.motor_l_pwm;
  document.getElementById('t-rpwm').textContent  = d.motor_r_pwm;
  document.getElementById('t-bat').textContent   = d.battery_v.toFixed(2) + 'V (' + d.battery_pct + '%)';
  document.getElementById('t-rssi').textContent  = d.wifi_rssi + ' dBm';
  document.getElementById('t-heap').textContent  = (d.free_heap/1024).toFixed(0) + ' kB';
  document.getElementById('batt-indicator').textContent = d.battery_pct + '%';

  const s = d.uptime_s % 60, m = Math.floor(d.uptime_s/60) % 60, h = Math.floor(d.uptime_s/3600);
  document.getElementById('uptime-display').textContent =
    String(h).padStart(2,'0') + ':' + String(m).padStart(2,'0') + ':' + String(s).padStart(2,'0');

  // Attitude canvas
  drawAttitude(d.angle);

  // PID chart
  telemetryHistory.push(d.angle);
  if (telemetryHistory.length > histMax) telemetryHistory.shift();
  drawChart();

  // Update PID slider labels from telemetry
  if (d.pid_angle) {
    document.getElementById('a-kp').value = d.pid_angle.kp;
    document.getElementById('a-ki').value = d.pid_angle.ki;
    document.getElementById('a-kd').value = d.pid_angle.kd;
    document.getElementById('r-kp').value = d.pid_rate.kp;
    document.getElementById('r-ki').value = d.pid_rate.ki;
    document.getElementById('r-kd').value = d.pid_rate.kd;
    updatePidLabels();
  }

  // AI conversation state
  if (d.conv_state !== undefined) {
    const states = ['待机','聆听中','ASR中','思考中','说话中','显示回复','出错'];
    const el = document.getElementById('conv-status');
    if (el) {
      el.textContent = states[d.conv_state] || '未知';
      el.className = d.conv_state === 0 ? 'conv-idle' : '';
    }
  }
  if (d.last_reply && d.last_reply.length > 0 && d.conv_state === 5) {
    addChatMsg('assistant', d.last_reply);
  }
}

// ===== Attitude Canvas =====
function drawAttitude(angleDeg) {
  const c = document.getElementById('attitude-canvas');
  if (!c) return;
  const ctx = c.getContext('2d');
  const w = c.width, h = c.height, cx = w/2, cy = h/2;

  ctx.clearRect(0, 0, w, h);

  // 地面线
  ctx.strokeStyle = '#555'; ctx.lineWidth = 2;
  ctx.beginPath(); ctx.moveTo(20, cy+40); ctx.lineTo(w-20, cy+40); ctx.stroke();

  // 机器人身体 (旋转)
  const ang = angleDeg * Math.PI / 180;
  ctx.save();
  ctx.translate(cx, cy+40);
  ctx.rotate(ang);
  ctx.translate(0, -40);

  // 轮子
  ctx.fillStyle = '#333';
  ctx.fillRect(-35, -8, 16, 16);
  ctx.fillRect(19, -8, 16, 16);

  // 身体
  ctx.fillStyle = '#555';
  ctx.fillRect(-20, -50, 40, 40);

  // 显示屏
  ctx.fillStyle = '#222';
  ctx.fillRect(-22, -55, 44, 30);
  ctx.fillStyle = '#0af';
  ctx.fillRect(-14, -50, 28, 18);

  // 头
  ctx.fillStyle = '#eee';
  ctx.beginPath(); ctx.arc(0, -70, 18, 0, Math.PI*2); ctx.fill();

  ctx.restore();

  // 角度指示
  ctx.fillStyle = '#fff'; ctx.font = '14px monospace';
  ctx.fillText(angleDeg.toFixed(1) + '°', cx-20, h-10);
}

// ===== PID Chart =====
function drawChart() {
  const c = document.getElementById('pid-chart');
  if (!c || telemetryHistory.length < 2) return;
  const ctx = c.getContext('2d');
  const w = c.width, h = c.height;

  ctx.fillStyle = getComputedStyle(document.body).getPropertyValue('--card').trim();
  ctx.fillRect(0, 0, w, h);

  // 零线
  ctx.strokeStyle = '#444'; ctx.lineWidth = 1;
  ctx.beginPath(); ctx.moveTo(0, h/2); ctx.lineTo(w, h/2); ctx.stroke();

  // 角度曲线
  ctx.strokeStyle = '#00d4aa'; ctx.lineWidth = 2;
  ctx.beginPath();
  const scaleY = 15; // pixels per degree
  for (let i = 0; i < telemetryHistory.length; i++) {
    const x = (i / histMax) * w;
    const y = h/2 - telemetryHistory[i] * scaleY;
    if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
  }
  ctx.stroke();

  // 标签
  ctx.fillStyle = '#888'; ctx.font = '10px monospace';
  ctx.fillText('俯仰角历史 (' + telemetryHistory.length + 'pts)', 4, 14);
}

// ===== PID Sliders =====
function updatePidLabels() {
  document.getElementById('a-kp-v').textContent = document.getElementById('a-kp').value;
  document.getElementById('a-ki-v').textContent = document.getElementById('a-ki').value;
  document.getElementById('a-kd-v').textContent = document.getElementById('a-kd').value;
  document.getElementById('r-kp-v').textContent = document.getElementById('r-kp').value;
  document.getElementById('r-ki-v').textContent = document.getElementById('r-ki').value;
  document.getElementById('r-kd-v').textContent = document.getElementById('r-kd').value;
}

['a-kp','a-ki','a-kd','r-kp','r-ki','r-kd'].forEach(id => {
  const el = document.getElementById(id);
  el.addEventListener('input', () => {
    updatePidLabels();
    const loop = id.startsWith('a-') ? 'angle' : 'rate';
    send({cmd:'pid_set', params:{loop, kp:parseFloat(document.getElementById('a-kp').value),
      ki:parseFloat(document.getElementById('a-ki').value),
      kd:parseFloat(document.getElementById('a-kd').value)}});
    // Also update rate params
    send({cmd:'pid_set', params:{loop:'rate', kp:parseFloat(document.getElementById('r-kp').value),
      ki:parseFloat(document.getElementById('r-ki').value),
      kd:parseFloat(document.getElementById('r-kd').value)}});
  });
});

document.getElementById('btn-pid-save').addEventListener('click', () => {
  send({cmd:'pid_save'});
  alert('PID 参数已保存到 Flash');
});

// ===== Tabs =====
document.querySelectorAll('.tab').forEach(t => {
  t.addEventListener('click', () => {
    document.querySelectorAll('.tab,.tab-content').forEach(e => e.classList.remove('active'));
    t.classList.add('active');
    document.getElementById('tab-' + t.dataset.tab).classList.add('active');
  });
});

// ===== Faces =====
document.querySelectorAll('.face-btn').forEach(b => {
  b.addEventListener('click', () => {
    const exprNames = ['neutral','happy','sad','angry','surprised','wink','love','sleep','confused','excited'];
    send({cmd:'expression', params:{type: exprNames[parseInt(b.dataset.expr)]}});
  });
});

// ===== DPAD =====
document.querySelectorAll('.dpad-btn').forEach(b => {
  const dirMap = { forward:'forward', backward:'backward', left:'left', right:'right', stop:'stop' };
  b.addEventListener('pointerdown', (e) => {
    e.preventDefault();
    const dir = dirMap[b.dataset.dir];
    if (dir === 'stop') send({cmd:'stop'});
    else {
      const speed = parseInt(document.getElementById('speed-slider').value);
      const turn = (dir === 'left') ? -80 : (dir === 'right') ? 80 : 0;
      const fwd = (dir === 'backward') ? -speed : (dir === 'forward') ? speed : 0;
      send({cmd:'move', params:{direction:dir, speed, turn, duration_ms:2000}});
    }
  });
  b.addEventListener('pointerup', () => {
    if (b.dataset.dir !== 'stop') send({cmd:'stop'});
  });
});

document.getElementById('speed-slider').addEventListener('input', function() {
  document.getElementById('speed-val').textContent = this.value;
});

// Keyboard
document.addEventListener('keydown', (e) => {
  const speed = parseInt(document.getElementById('speed-slider').value);
  const map = {
    'w': ['forward', speed], 's': ['backward', speed],
    'a': ['left', 80], 'd': ['right', 80]
  };
  if (map[e.key]) {
    const [dir, spd] = map[e.key];
    send({cmd:'move', params:{direction:dir, speed:spd, turn:(dir==='left'?-80:dir==='right'?80:0), duration_ms:500}});
  }
  if (e.key === ' ') { e.preventDefault(); send({cmd:'stop'}); }
});
document.addEventListener('keyup', (e) => {
  if (['w','a','s','d'].includes(e.key)) send({cmd:'stop'});
});

// ===== Quick controls =====
document.getElementById('btn-stand').addEventListener('click', () => send({cmd:'stop'}));
document.getElementById('btn-rest').addEventListener('click', () => {
  send({cmd:'expression', params:{type:'sleep'}});
  send({cmd:'animation', params:{name:'crouch'}});
});
document.getElementById('btn-clear').addEventListener('click', () => send({cmd:'stop'}));

// ===== Servo sliders =====
document.querySelectorAll('.servo-slider').forEach(s => {
  s.addEventListener('input', function() {
    const ch = parseInt(this.dataset.ch);
    const ang = parseInt(this.value);
    document.querySelector('.servo-val[data-ch="'+ch+'"]').textContent = ang + '°';
    send({cmd:'servo', params:{channel:ch, angle:ang, speed:100}});
  });
});

// ===== System =====
document.getElementById('btn-refresh-log').addEventListener('click', async () => {
  const resp = await fetch('/api/log');
  document.getElementById('log-view').textContent = await resp.text();
});
document.getElementById('btn-clear-log').addEventListener('click', () => {
  document.getElementById('log-view').textContent = '';
});
document.getElementById('btn-restart').addEventListener('click', () => {
  if (confirm('确定要重启 PenguinBot 吗?')) {
    fetch('/api/restart', {method:'POST'});
  }
});

// ===== Theme =====
document.getElementById('theme-toggle').addEventListener('click', () => {
  const html = document.documentElement;
  const cur = html.getAttribute('data-theme');
  html.setAttribute('data-theme', cur === 'light' ? 'dark' : 'light');
  localStorage.setItem('theme', cur === 'light' ? 'dark' : 'light');
});

// ===== AI Chat =====
function addChatMsg(role, text) {
  const div = document.createElement('div');
  div.className = 'chat-msg ' + role;
  div.textContent = text;
  const container = document.getElementById('chat-messages');
  container.appendChild(div);
  container.parentElement.scrollTop = container.parentElement.scrollHeight;
}

function sendChat() {
  const input = document.getElementById('chat-input');
  const text = input.value.trim();
  if (!text) return;
  addChatMsg('user', text);
  input.value = '';
  send({cmd:'chat', params:{text}});
}

// Enter to send
document.getElementById('chat-input').addEventListener('keydown', (e) => {
  if (e.key === 'Enter') sendChat();
});
document.getElementById('btn-chat-send').addEventListener('click', sendChat);

// Clear history
document.getElementById('btn-chat-clear').addEventListener('click', () => {
  send({cmd:'chat_clear'});
  document.getElementById('chat-messages').innerHTML =
    '<div class="chat-msg system">对话已清空，开始新对话吧~</div>';
});

// Trigger voice dialog
document.getElementById('btn-chat-trigger').addEventListener('click', () => {
  send({cmd:'stop'});  // 先让机器人站稳
  addChatMsg('system', '正在聆听...对着 PenguinBot 说话吧');
});

// Save API config
document.getElementById('btn-cfg-save').addEventListener('click', () => {
  const llmKey = document.getElementById('cfg-llm-key').value.trim();
  if (llmKey) {
    send({cmd:'ai_config', params:{key:'llm_key', value:llmKey}});
    alert('API Key 已保存');
  }
});

// Init
connectWS();
updatePidLabels();

// Load saved theme
const saved = localStorage.getItem('theme');
if (saved) document.documentElement.setAttribute('data-theme', saved);
