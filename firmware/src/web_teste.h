#ifndef WEB_TESTE_H
#define WEB_TESTE_H

const char* WEB_TESTE_HTML = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Painel I-V | Traçador</title>
    <style>
        :root {
            --bg-dark: #0f172a;
            --panel-bg: #1e293b;
            --primary: #3b82f6;
            --primary-hover: #2563eb;
            --accent: #f59e0b;
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
            --success: #10b981;
            --danger: #ef4444;
        }

        * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Segoe UI', system-ui, sans-serif; }
        
        body {
            background-color: var(--bg-dark);
            color: var(--text-main);
            min-height: 100vh;
            padding: 2rem;
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        .header {
            width: 100%;
            max-width: 1000px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 2rem;
            padding-bottom: 1rem;
            border-bottom: 1px solid #334155;
        }

        .nav-links a {
            color: var(--text-muted);
            text-decoration: none;
            margin-left: 1.5rem;
            font-weight: 600;
            transition: color 0.2s;
        }
        .nav-links a:hover { color: var(--primary); }
        .nav-links a.active { color: var(--text-main); }

        .container {
            width: 100%;
            max-width: 1000px;
            display: grid;
            grid-template-columns: 1fr 3fr;
            gap: 2rem;
        }

        @media (max-width: 768px) {
            .container { grid-template-columns: 1fr; }
        }

        .glass-panel {
            background: rgba(30, 41, 59, 0.7);
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255,255,255,0.1);
            border-radius: 16px;
            padding: 1.5rem;
            box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
        }

        h2 { font-size: 1.2rem; margin-bottom: 1rem; color: var(--primary); }

        button {
            width: 100%;
            padding: 1rem;
            border: none;
            border-radius: 8px;
            background: linear-gradient(135deg, var(--primary), #1d4ed8);
            color: white;
            font-weight: bold;
            font-size: 1.1rem;
            cursor: pointer;
            transition: transform 0.2s, box-shadow 0.2s;
            margin-bottom: 1rem;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 0.5rem;
        }
        button:hover { transform: translateY(-2px); box-shadow: 0 10px 15px -3px rgba(59, 130, 246, 0.4); }
        button:disabled { background: #475569; cursor: not-allowed; transform: none; box-shadow: none; }

        .btn-secondary {
            background: transparent;
            border: 1px solid var(--primary);
            color: var(--primary);
            font-size: 0.9rem;
            padding: 0.75rem;
        }
        .btn-secondary:hover { background: rgba(59, 130, 246, 0.1); }

        .status-badge {
            display: inline-block;
            padding: 0.25rem 0.75rem;
            border-radius: 999px;
            font-size: 0.85rem;
            font-weight: bold;
            background: #334155;
            color: var(--text-main);
            margin-bottom: 1rem;
            width: 100%;
            text-align: center;
        }
        
        .status-idle { background: rgba(16, 185, 129, 0.2); color: var(--success); }
        .status-running { background: rgba(245, 158, 11, 0.2); color: var(--accent); }

        .progress-bar-bg {
            width: 100%;
            height: 8px;
            background: #334155;
            border-radius: 4px;
            overflow: hidden;
            margin-bottom: 1.5rem;
            display: none;
        }
        .progress-bar-fill {
            height: 100%;
            background: var(--accent);
            width: 0%;
            transition: width 0.3s linear;
        }

        .chart-container {
            width: 100%;
            height: 400px;
            position: relative;
        }

        .file-upload {
            margin-top: 2rem;
            padding-top: 1.5rem;
            border-top: 1px dashed #334155;
        }
        input[type="file"] {
            display: none;
        }
        .file-label {
            display: block;
            text-align: center;
            padding: 1rem;
            border: 1px dashed var(--text-muted);
            border-radius: 8px;
            color: var(--text-muted);
            cursor: pointer;
            transition: all 0.2s;
        }
        .file-label:hover { border-color: var(--primary); color: var(--primary); }

    </style>
</head>
<body>

    <div class="header">
        <h1>⚡ Traçador de Curva I-V</h1>
        <div class="nav-links">
            <a href="/" class="active">Painel de Teste</a>
            <a href="/ajustes">Ajustes & Calibração</a>
        </div>
    </div>

    <div class="container">
        <!-- Barra Lateral de Controles -->
        <div class="glass-panel">
            <h2>Controles</h2>
            
            <div id="statusBadge" class="status-badge status-idle">Status: PRONTO</div>
            
            <div class="progress-bar-bg" id="progressBg">
                <div class="progress-bar-fill" id="progressFill"></div>
            </div>

            <div style="margin-bottom: 1.5rem; background: rgba(0,0,0,0.2); padding: 1rem; border-radius: 8px;">
                <label for="freqSlider" style="display: block; margin-bottom: 0.5rem; font-size: 0.9rem; color: var(--text-muted);">
                    Frequência PWM: <span id="freqVal" style="color: var(--accent); font-weight: bold; font-size: 1.1rem;">15</span> kHz
                </label>
                <input type="range" id="freqSlider" min="1" max="100" value="15" style="width: 100%; accent-color: var(--accent);" onchange="changeFreq(this.value)" oninput="document.getElementById('freqVal').innerText = this.value">
                <small style="color: #64748b; font-size: 0.75rem; display: block; margin-top: 0.5rem;">Sintonize ao vivo (sem reiniciar)</small>
            </div>

            <div style="margin-bottom: 1.5rem; background: rgba(0,0,0,0.2); padding: 1rem; border-radius: 8px;">
                <label style="display: block; margin-bottom: 0.75rem; font-size: 0.9rem; color: var(--text-muted); font-weight: bold;">Zoom de Duty Cycle (%)</label>
                <div style="display: flex; gap: 0.5rem; margin-bottom: 0.5rem;">
                    <div style="flex: 1;">
                        <label style="font-size: 0.75rem; color: var(--text-muted);">Início (Máx %)</label>
                        <input type="number" id="pwmMax" value="80" min="0" max="100" style="width: 100%; padding: 0.4rem; border-radius: 4px; border: 1px solid #334155; background: #1e293b; color: white;">
                    </div>
                    <div style="flex: 1;">
                        <label style="font-size: 0.75rem; color: var(--text-muted);">Fim (Mín %)</label>
                        <input type="number" id="pwmMin" value="40" min="0" max="100" style="width: 100%; padding: 0.4rem; border-radius: 4px; border: 1px solid #334155; background: #1e293b; color: white;">
                    </div>
                </div>
                <small style="color: #64748b; font-size: 0.75rem; display: block; margin-top: 0.5rem;">O teste sempre gerará 20 pontos de alta resolução (~10 seg)</small>
            </div>

            <button id="btnRunTest" onclick="startTest()">
                <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polygon points="5 3 19 12 5 21 5 3"></polygon></svg>
                Rodar Teste I-V
            </button>

            <a href="/download_csv" style="text-decoration: none;">
                <button class="btn-secondary">
                    <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="7 10 12 15 17 10"/><line x1="12" y1="15" x2="12" y2="3"/></svg>
                    Baixar CSV do SD
                </button>
            </a>

            <div class="file-upload">
                <h2 style="font-size: 1rem;">Analisar CSV Local</h2>
                <p style="font-size: 0.8rem; color: var(--text-muted); margin-bottom: 1rem;">Faça upload de um arquivo CSV gerado anteriormente para plotar o gráfico aqui.</p>
                <label class="file-label">
                    <input type="file" id="csvFileInput" accept=".csv" onchange="handleFileUpload(event)">
                    Clique para selecionar o arquivo .csv
                </label>
            </div>
        </div>

        <!-- Área do Gráfico -->
        <div class="glass-panel" style="display: flex; flex-direction: column;">
            <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 1rem;">
                <h2>Gráfico Tensão x Corrente (I-V)</h2>
            </div>
            
            <div class="chart-container">
                <canvas id="ivChart"></canvas>
            </div>
        </div>
    </div>

    <script>
        let pollingInterval = null;
        const canvas = document.getElementById('ivChart');
        const ctx = canvas.getContext('2d');
        let currentPlotData = [];
        let lastTestTime = "";
        let lastTestFreq = "";

        function drawChart(data) {
            const w = canvas.width = canvas.parentElement.clientWidth;
            const h = canvas.height = canvas.parentElement.clientHeight;
            ctx.clearRect(0, 0, w, h);
            
            const padding = 40;
            const plotW = w - padding * 2;
            const plotH = h - padding * 2;
            
            // Fundo
            ctx.fillStyle = 'rgba(255,255,255,0.02)';
            ctx.fillRect(padding, padding, plotW, plotH);
            
            // Eixos
            ctx.beginPath();
            ctx.strokeStyle = '#94a3b8';
            ctx.lineWidth = 2;
            ctx.moveTo(padding, padding);
            ctx.lineTo(padding, h - padding);
            ctx.lineTo(w - padding, h - padding);
            ctx.stroke();
            
            // Textos dos Eixos
            ctx.fillStyle = '#94a3b8';
            ctx.font = '13px sans-serif';
            ctx.textAlign = 'center';
            ctx.fillText('Tensão (V)', w / 2, h - 5);
            ctx.save();
            ctx.translate(15, h / 2);
            ctx.rotate(-Math.PI / 2);
            ctx.fillText('Corrente (A)', 0, 0);
            ctx.restore();

            // Metadados do Teste (Canto Superior Direito)
            if (lastTestTime !== "") {
                ctx.textAlign = 'right';
                ctx.fillStyle = 'rgba(255, 255, 255, 0.7)';
                ctx.font = 'bold 12px sans-serif';
                ctx.fillText(`Horário: ${lastTestTime} | Freq: ${lastTestFreq} kHz`, w - padding, padding + 15);
            }

            if (data.length === 0) {
                ctx.fillStyle = '#64748b';
                ctx.fillText('Sem dados válidos (V=0, I=0)', w/2, h/2);
                return;
            }

            // Escalas Fixas para manter proporção entre testes e ticks inteiros (5V e 1A)
            const maxX = 25; 
            const maxY = 5;
            
            // Grid e Valores Múltiplos
            ctx.fillStyle = '#94a3b8';
            ctx.strokeStyle = 'rgba(255,255,255,0.05)';
            ctx.lineWidth = 1;
            
            const numTicks = 5;
            
            // Eixo Y (linhas horizontais)
            ctx.textAlign = 'right';
            for (let i = 0; i <= numTicks; i++) {
                const val = (maxY / numTicks) * i;
                const py = (h - padding) - (val / maxY) * plotH;
                
                if (i > 0) {
                    ctx.beginPath();
                    ctx.moveTo(padding, py);
                    ctx.lineTo(w - padding, py);
                    ctx.stroke();
                }
                ctx.fillText(val.toFixed(1), padding - 5, py + 4);
            }
            
            // Eixo X (linhas verticais)
            ctx.textAlign = 'center';
            for (let i = 1; i <= numTicks; i++) {
                const val = (maxX / numTicks) * i;
                const px = padding + (val / maxX) * plotW;
                
                ctx.beginPath();
                ctx.moveTo(px, padding);
                ctx.lineTo(px, h - padding);
                ctx.stroke();
                
                ctx.fillText(val.toFixed(1), px, h - padding + 15);
            }

            // Linha Sombra Fantasma (Potenciômetro Físico - Ground Truth)
            const baselineData = [{x:0.92, y:5.41}, {x:1.94, y:5.42}, {x:3.65, y:5.39}, {x:7.56, y:5.35}, {x:10.04, y:5.33}, {x:11.59, y:5.25}, {x:12.94, y:5.02}, {x:14.5, y:4.5}, {x:15.37, y:4.06}, {x:16.06, y:3.6}, {x:16.56, y:3.25}, {x:16.85, y:3.0}, {x:17.24, y:2.67}, {x:17.42, y:2.52}, {x:17.61, y:2.35}, {x:17.83, y:2.25}, {x:18.05, y:1.96}, {x:18.31, y:1.71}, {x:18.53, y:1.48}, {x:18.76, y:1.25}, {x:18.9, y:1.09}, {x:18.94, y:0.95}];
            ctx.beginPath();
            ctx.strokeStyle = 'rgba(255, 255, 255, 0.25)'; // Sombra branca transparente
            ctx.lineWidth = 3;
            ctx.setLineDash([5, 5]); // Linha tracejada
            baselineData.forEach((p, i) => {
                const px = padding + (p.x / maxX) * plotW;
                const py = (h - padding) - (p.y / maxY) * plotH;
                if(i === 0) ctx.moveTo(px, py);
                else ctx.lineTo(px, py);
            });
            ctx.stroke();
            ctx.setLineDash([]); // Reseta para a linha normal do IGBT

            // Linha do gráfico do IGBT (Teste Atual)
            ctx.beginPath();
            ctx.strokeStyle = '#3b82f6';
            ctx.lineWidth = 3;
            data.forEach((p, i) => {
                const px = padding + (p.x / maxX) * plotW;
                const py = (h - padding) - (p.y / maxY) * plotH;
                if(i === 0) ctx.moveTo(px, py);
                else ctx.lineTo(px, py);
            });
            ctx.stroke();
            
            // Pontos
            ctx.fillStyle = '#f59e0b';
            data.forEach(p => {
                const px = padding + (p.x / maxX) * plotW;
                const py = (h - padding) - (p.y / maxY) * plotH;
                ctx.beginPath();
                ctx.arc(px, py, 4, 0, Math.PI * 2);
                ctx.fill();
            });
        }

        window.addEventListener('resize', () => drawChart(currentPlotData));
        drawChart([]); // Inicia vazio

        // Sintonizador de Frequência Dinâmico
        function changeFreq(valKhz) {
            const freqHz = valKhz * 1000;
            fetch('/set_freq?val=' + freqHz)
            .then(res => {
                if(!res.ok) alert("Falha ao comunicar a frequência com a placa.");
            }).catch(err => alert("Sem conexão com o ESP32."));
        }

        // Inicia teste
        function startTest() {
            const pMax = document.getElementById('pwmMax').value;
            const pMin = document.getElementById('pwmMin').value;

            document.getElementById('btnRunTest').disabled = true;
            document.getElementById('statusBadge').className = 'status-badge status-running';
            document.getElementById('statusBadge').innerText = 'Status: PREPARANDO...';
            document.getElementById('progressBg').style.display = 'block';
            document.getElementById('progressFill').style.width = '0%';
            
            fetch(`/start_test?start=${pMax}&end=${pMin}`, { method: 'POST' })
            .then(res => res.text())
            .then(txt => {
                if(txt === "OK") {
                    // Começa a pesquisar o status a cada 500ms
                    pollingInterval = setInterval(pollStatus, 500);
                } else {
                    alert("Erro ao iniciar: " + txt);
                    resetUI();
                }
            }).catch(err => {
                alert("Erro de conexão");
                resetUI();
            });
        }

        // Verifica andamento
        function pollStatus() {
            fetch('/status')
            .then(res => res.json())
            .then(data => {
                const state = data.state;
                if(state === "SWEEPING") {
                    document.getElementById('statusBadge').innerText = 'Status: VARREDURA (' + data.step + '/256)';
                    document.getElementById('progressFill').style.width = (data.step * (100 / 256)) + '%';
                } else if(state === "SAVING") {
                    document.getElementById('statusBadge').innerText = 'Status: SALVANDO SD...';
                    document.getElementById('progressFill').style.width = '100%';
                } else if(state === "IDLE") {
                    // Teste acabou
                    clearInterval(pollingInterval);
                    resetUI();
                    loadLastTest();
                }
            }).catch(err => console.error("Erro poll:", err));
        }

        function resetUI() {
            document.getElementById('btnRunTest').disabled = false;
            document.getElementById('statusBadge').className = 'status-badge status-idle';
            document.getElementById('statusBadge').innerText = 'Status: PRONTO';
            document.getElementById('progressBg').style.display = 'none';
        }

        // Puxa array JSON com os 20 pontos recém gerados
        function loadLastTest() {
            fetch('/last_test')
            .then(res => res.json())
            .then(data => {
                // data é array de {v: 12.3, i: 2.1}
                updateChart(data);
            });
        }

        function updateChart(dataPoints) {
            const plotData = dataPoints.filter(d => d.v > 0 || d.i > 0).map(d => ({ x: d.v, y: d.i }));
            plotData.sort((a,b) => a.x - b.x); // Ordena por Tensão
            currentPlotData = plotData;
            
            // Puxa o horário e a frequência no momento que o gráfico é plotado
            lastTestTime = new Date().toLocaleTimeString('pt-BR', { hour: '2-digit', minute: '2-digit' });
            lastTestFreq = document.getElementById('freqVal').innerText;
            
            drawChart(plotData);
        }

        // Função para upar e parsear CSV local
        function handleFileUpload(event) {
            const file = event.target.files[0];
            if (!file) return;

            const reader = new FileReader();
            reader.onload = function(e) {
                const text = e.target.result;
                const lines = text.split('\n');
                let points = [];
                
                // Pula cabeçalho, assume formato: Data;Tensao_V;Corrente_A;...
                for(let i = 1; i < lines.length; i++) {
                    if(lines[i].trim() === '') continue;
                    const cols = lines[i].split(';');
                    if(cols.length >= 3) {
                        const v = parseFloat(cols[1].replace(',','.'));
                        const i_val = parseFloat(cols[2].replace(',','.'));
                        if(!isNaN(v) && !isNaN(i_val)) {
                            points.push({v: v, i: i_val});
                        }
                    }
                }
                
                if(points.length > 0) {
                    lastTestTime = "Arquivo Local";
                    lastTestFreq = "?";
                    updateChart(points);
                    alert("Arquivo carregado e plotado com sucesso!");
                } else {
                    alert("Não foi possível encontrar dados válidos de Tensão e Corrente no CSV.");
                }
            };
            reader.readAsText(file);
        }
    </script>
</body>
</html>
)rawliteral";

#endif
