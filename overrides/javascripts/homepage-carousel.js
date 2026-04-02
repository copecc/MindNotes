document$.subscribe(() => {
  const tracks = document.querySelectorAll("[data-scroll-track]");

  tracks.forEach((track) => {
    const trackId = track.id;
    if (!trackId) return;
    const strip = track.closest(".home-strip");
    if (!strip) return;

    const controls = document.querySelectorAll(`[data-scroll-target="${trackId}"]`);
    const updateState = () => {
      const maxScroll = track.scrollWidth - track.clientWidth;
      const atStart = track.scrollLeft <= 4;
      const atEnd = maxScroll <= 4 || track.scrollLeft >= maxScroll - 4;
      const canScroll = maxScroll > 4;

      strip.classList.toggle("is-at-start", atStart || !canScroll);
      strip.classList.toggle("is-at-end", atEnd || !canScroll);
      strip.classList.toggle("is-static", !canScroll);

      controls.forEach((control) => {
        const direction = Number(control.dataset.scrollDirection || "1");
        const disabled = !canScroll || (direction < 0 ? atStart : atEnd);
        control.disabled = disabled;
        control.setAttribute("aria-disabled", disabled ? "true" : "false");
      });
    };

    controls.forEach((control) => {
      control.addEventListener("click", () => {
        if (control.disabled) return;
        const direction = Number(control.dataset.scrollDirection || "1");
        const firstCard = track.querySelector(".home-card");
        const step = firstCard ? firstCard.getBoundingClientRect().width + 16 : track.clientWidth * 0.8;
        track.scrollBy({ left: direction * step, behavior: "smooth" });
      });
    });

    track.addEventListener("scroll", updateState, { passive: true });
    window.addEventListener("resize", updateState);
    updateState();
  });
});
