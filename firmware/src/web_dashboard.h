#ifndef WEB_DASHBOARD_H
#define WEB_DASHBOARD_H

const char* WEB_DASHBOARD_HTML = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Painel I-V Solar</title>
    <style>
        :root {
            --bg-color: #0f172a;
            --panel-bg: rgba(30, 41, 59, 0.7);
            --primary: #3b82f6;
            --accent: #8b5cf6;
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
            --border: rgba(255, 255, 255, 0.1);
        }

        body {
            background-color: var(--bg-color);
            color: var(--text-main);
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            margin: 0;
            padding: 2rem;
            display: flex;
            justify-content: center;
            min-height: 100vh;
        }

        .container {
            max-width: 1200px;
            width: 100%;
            display: grid;
            grid-template-columns: 350px 1fr;
            gap: 2rem;
        }

        .glass-panel {
            background: var(--panel-bg);
            backdrop-filter: blur(12px);
            -webkit-backdrop-filter: blur(12px);
            border: 1px solid var(--border);
            border-radius: 20px;
            padding: 2rem;
            box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.5);
        }

        h1, h2 { margin-top: 0; }
        
        h1 {
            font-size: 1.5rem;
            background: linear-gradient(to right, var(--primary), var(--accent));
            -webkit-background-clip: text;
            color: transparent;
            margin-bottom: 2rem;
        }

        .metric {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 1rem 0;
            border-bottom: 1px solid var(--border);
        }

        .metric:last-child { border-bottom: none; }
        
        .metric-value {
            font-size: 1.5rem;
            font-weight: 600;
            color: var(--primary);
        }

        input {
            background: rgba(0,0,0,0.3);
            border: 1px solid var(--border);
            color: white;
            padding: 0.5rem;
            border-radius: 8px;
            width: 100px;
            text-align: right;
            outline: none;
            transition: 0.3s;
        }
        
        input:focus { border-color: var(--accent); }

        button {
            width: 100%;
            padding: 1rem;
            border: none;
            border-radius: 12px;
            background: linear-gradient(135deg, var(--primary), var(--accent));
            color: white;
            font-size: 1rem;
            font-weight: 600;
            cursor: pointer;
            transition: transform 0.2s, box-shadow 0.2s;
            margin-top: 1.5rem;
        }

        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 10px 20px -10px var(--accent);
        }

        .calibration-section {
            margin-top: 2rem;
            padding-top: 2rem;
            border-top: 1px dashed var(--border);
        }
    </style>
</head>
<body>

    <div class="header">
        <h1>⚡ Traçador de Curva I-V</h1>
        <div class="nav-links">
            <a href="/">Painel de Teste</a>
            <a href="/ajustes" class="active">Ajustes & Calibração</a>
        </div>
    </div>

    <div class="container">
        <div class="glass-panel">
            <h1>Controle I-V</h1>
            
            <div>
                <div class="metric">
                    <span style="color: var(--text-muted);">Tensão Lida (V)</span>
                    <span class="metric-value" id="live_v">0.0</span>
                </div>
                <div class="metric">
                    <span style="color: var(--text-muted);">Corrente Lida (A)</span>
                    <span class="metric-value" id="live_i">0.00</span>
                </div>
            </div>

            <div class="calibration-section">
                <h2>Calibração Real</h2>
                <p style="font-size: 0.85rem; color: var(--text-muted); margin-bottom: 1rem;">Digite o valor real medido no multímetro para calcular o multiplicador e salvar no ESP32.</p>
                
                <div class="metric">
                    <span>Tensão Real</span>
                    <input type="number" id="real_v" step="0.1" placeholder="Ex: 12.0">
                </div>
                <div class="metric">
                    <span>Corrente Real</span>
                    <input type="number" id="real_i" step="0.01" placeholder="Ex: 5.50">
                </div>
                <button onclick="calibrate()">Calibrar e Salvar</button>
            </div>

            <div class="calibration-section">
                <h2>Carga Manual (PWM)</h2>
                <p style="font-size: 0.85rem; color: var(--text-muted); margin-bottom: 1rem;">Ajuste manualmente a carga do IGBT (0 a 100%). Só funciona se um teste não estiver rodando.</p>
                
                <div style="display:flex; gap:1rem; align-items:center;">
                    <input type="range" id="pwm_slider" min="0" max="100" value="0" style="flex:1;" oninput="document.getElementById('pwm_val_display').innerText = this.value + '%'">
                    <span id="pwm_val_display" style="width: 50px; text-align: right; color: var(--primary); font-weight: bold;">0%</span>
                </div>
                <button onclick="setPWM()" style="background: linear-gradient(135deg, #10b981, #059669);">Aplicar Carga</button>
            </div>
        </div>

        <div class="glass-panel" style="display:flex; align-items:center; justify-content:center; flex-direction:column;">
            <div style="text-align:center; color:var(--text-muted);">
                <i>(A área do Gráfico interativo e do Teste I-V será adicionada no próximo passo!)</i>
            </div>
        </div>
    </div>

    <script>
        // Atualiza os valores ao vivo a cada 2 segundos (tempo do Modbus IDLE)
        setInterval(() => {
            fetch('/live')
                .then(res => res.json())
                .then(data => {
                    document.getElementById('live_v').innerText = parseFloat(data.v).toFixed(1);
                    document.getElementById('live_i').innerText = parseFloat(data.i).toFixed(2);
                })
                .catch(err => console.error(err));
        }, 2000);

        function calibrate() {
            const real_v = document.getElementById('real_v').value;
            const real_i = document.getElementById('real_i').value;
            
            const formData = new URLSearchParams();
            if(real_v) formData.append('real_v', real_v);
            if(real_i) formData.append('real_i', real_i);

            if(!real_v && !real_i) {
                alert("Preencha ao menos um valor real para calibrar.");
                return;
            }

            fetch('/calibrate', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: formData.toString()
            })
            .then(res => res.text())
            .then(txt => {
                alert("Calibrado com sucesso! Os multiplicadores foram salvos.");
                document.getElementById('real_v').value = '';
                document.getElementById('real_i').value = '';
            })
            .catch(err => alert("Erro ao calibrar!"));
        }

        function setPWM() {
            const val = document.getElementById('pwm_slider').value;
            fetch('/set_pwm', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: 'val=' + val
            })
            .then(res => res.text())
            .then(txt => alert("PWM Ajustado para " + val + "%"))
            .catch(err => alert("Erro ao enviar PWM"));
        }
    </script>
</body>
</html>
)rawliteral";

#endif
