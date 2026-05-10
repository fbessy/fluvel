// ===== ICON =====
function copyIcon() {
  return `
  <svg viewBox="0 0 24 24">
    <path d="M8 1h12a2 2 0 0 1 2 2v12h-2V3H8V1zm-3 4h11a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V7a2 2 0 0 1 2-2zm0 2v14h11V7H5z"/>
  </svg>`;
}

// ===== CHECK ICON =====
function checkIcon() {
  return `
  <svg viewBox="0 0 24 24">
    <path d="M9.2 16.2 4.8 11.8l-1.4 1.4 5.8 5.8 11-11-1.4-1.4z"/>
  </svg>`;
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
    return;
  }

  el.href = url;
  el.target = "_blank";
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

        const hash = parts.shift();
        const filename = parts.join(" ");

        const div = document.createElement("div");
        div.className = "copy-line";

        const hashText = document.createElement("span");
        hashText.className = "checksum-hash";
        hashText.innerText = hash;

        const fileText = document.createElement("span");
        fileText.className = "checksum-file";
        fileText.innerText = filename;

        const btn = document.createElement("button");
        btn.className = "copy-btn";
        btn.innerHTML = copyIcon();

        btn.onclick = async () => {

          try {

            await navigator.clipboard.writeText(hash);

            const oldIcon = btn.innerHTML;

            btn.innerHTML = checkIcon();
            btn.classList.add("copied");

            setTimeout(() => {
              btn.innerHTML = oldIcon;
              btn.classList.remove("copied");
            }, 1000);

          } catch (err) {

            console.error("Copy failed:", err);

          }
        };

        div.appendChild(hashText);
        div.appendChild(btn);
        div.appendChild(fileText);

        container.appendChild(div);
      });
    }

    // 🔥 IMPORTANT → après tout
    highlightPlatform();

  });
