// ===== ICON =====
function copyIcon() {
  return `
  <svg viewBox="0 0 16 16">
  <path d="M4 1h7a2 2 0 0 1 2 2v7h-1V3a1 1 0 0 0-1-1H4V1z"/>
  <path d="M2 4a2 2 0 0 1 2-2h7a2 2 0 0 1 2 2v7a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V4zm2-1a1 1 0 0 0-1 1v7a1 1 0 0 0 1 1h7a1 1 0 0 0 1-1V4a1 1 0 0 0-1-1H4z"/>
  </svg>`;
}

// ===== TOAST =====
const toast = document.getElementById("copy-toast");

function showToast() {
  toast.classList.add("show");
  setTimeout(() => toast.classList.remove("show"), 1200);
}

// ===== OS DETECTION =====
function detectOS() {
  const ua = navigator.userAgent.toLowerCase();

  if (ua.includes("win")) return "windows";
  if (ua.includes("mac")) return "macos";
  if (ua.includes("linux")) return "linux";

  return null; // important → pas de fallback
}

// ===== HIGHLIGHT =====
function highlightPlatform() {
  const os = detectOS();
  if (!os) return; // 👉 rien si non reconnu

  const section = document.getElementById("platform_" + os);
  if (!section) return;

  section.classList.add("highlight");

  // badge
  const badge = document.createElement("span");
  badge.className = "badge";
  badge.innerText = "Recommended";

  section.querySelector("h3").appendChild(badge);
}

// ===== LINKS =====
function setLink(id, url) {
  const el = document.getElementById(id);

  if (!url) {
    el.style.display = "none";
    return false;
  }

  el.href = "#";

  el.onclick = (e) => {
    e.preventDefault();
    window.open(url, "_blank");
  };

  return true;
}

// ===== PLATFORM VISIBILITY =====
function hidePlatformIfEmpty(platformId, linkIds) {
  const hasVisible = linkIds.some(id => {
    const el = document.getElementById(id);
    return el && el.style.display !== "none";
  });

  if (!hasVisible) {
    document.getElementById(platformId).style.display = "none";
  }
}

// ===== LOAD JSON =====
fetch("latest.json")
  .then(r => r.json())
  .then(data => {

    document.getElementById("version").innerText = data.version || "N/A";
    document.getElementById("branch").innerText = data.branch || "N/A";
    document.getElementById("commit").innerText =
      (data.commit || "N/A").slice(0, 7);

    if (data.date) {
      const d = new Date(data.date);
      document.getElementById("date").innerText =
        d.toLocaleString("en-GB", {
          year: "numeric",
          month: "long",
          day: "2-digit",
          hour: "2-digit",
          minute: "2-digit",
          second: "2-digit",
          timeZone: "UTC"
        }) + " UTC";
    }

    setLink("win_installer", data.windows?.installer);
    setLink("win_portable", data.windows?.portable);
    setLink("flatpak", data.linux?.flatpak);
    setLink("appimage", data.linux?.appimage);
    setLink("macos", data.macos?.dmg);

    hidePlatformIfEmpty("platform_windows", ["win_installer", "win_portable"]);
    hidePlatformIfEmpty("platform_linux", ["flatpak", "appimage"]);
    hidePlatformIfEmpty("platform_macos", ["macos"]);

    // ===== CHECKSUMS =====
    const container = document.getElementById("checksums");
    container.innerHTML = "";

    if (!data.checksums) {
      container.innerText = "No checksums available";
    } else {
      const lines = data.checksums.split("\n");

      lines.forEach(line => {
        if (!line.trim()) return;

        const parts = line.trim().split(/\s+/);
        const hash = parts[0];

        const div = document.createElement("div");
        div.className = "copy-line";

        const btn = document.createElement("button");
        btn.className = "copy-btn";
        btn.innerHTML = copyIcon();
        btn.title = "Copy hash";

        btn.onclick = () => {
          navigator.clipboard.writeText(hash);
          showToast();
        };

        const text = document.createElement("span");
        text.innerText = line;

        div.appendChild(btn);
        div.appendChild(text);
        container.appendChild(div);
      });
    }

    // 🔥 IMPORTANT → après tout
    highlightPlatform();

  });
