async function loadLogs() {
  try {
    const response = await fetch("http://localhost:8080/admin/logs");
    const logs = await response.json();

    renderStats(logs);
    renderTable(logs);
  } catch (error) {
    console.error("Failed to load logs:", error);
  }
}

function renderStats(logs) {
  document.getElementById("totalRequests").textContent = logs.length;

  if (logs.length === 0) {
    document.getElementById("avgLatency").textContent = "0 μs";
    document.getElementById("errorCount").textContent = "0";
    return;
  }

  const totalLatency = logs.reduce((sum, log) => sum + Number(log.latency), 0);
  const avgLatency = Math.round(totalLatency / logs.length);

  const errors = logs.filter(log => Number(log.status_code) >= 400).length;

  document.getElementById("avgLatency").textContent = avgLatency + " μs";
  document.getElementById("errorCount").textContent = errors;
}

function renderTable(logs) {
  const table = document.getElementById("logsTable");
  table.innerHTML = "";

  logs.forEach(log => {
    const row = document.createElement("tr");

    const statusClass = Number(log.status_code) >= 400
      ? "status-error"
      : "status-ok";

    row.innerHTML = `
      <td>${log.method}</td>
      <td>${log.origin_path}</td>
      <td>${log.target_host}</td>
      <td>${log.forward_path}</td>
      <td class="${statusClass}">${log.status_code}</td>
      <td>${log.headers_injected}</td>
      <td>${log.latency} μs</td>
      <td>${log.response_size} bytes</td>
    `;

    table.appendChild(row);
  });
}

loadLogs();
setInterval(loadLogs, 2000);