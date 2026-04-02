(() => {
  const normalizePath = (pathname) => {
    let path = pathname.replace(/index\.html?$/i, "");
    if (!path.endsWith("/")) path += "/";
    return path;
  };

  const prettifySegment = (segment) =>
    decodeURIComponent(segment)
      .replace(/[-_]+/g, " ")
      .replace(/\b\w/g, (char) => char.toUpperCase());

  const findSiblingPages = (dirPath, currentPath, recursive) => {
    const seen = new Set();
    const pages = [];

    document.querySelectorAll("a.md-nav__link[href]").forEach((anchor) => {
      const url = new URL(anchor.getAttribute("href"), window.location.href);
      const path = normalizePath(url.pathname);
      if (!path.startsWith(dirPath) || path === currentPath) return;

      const relative = path.slice(dirPath.length).replace(/^\/+/, "");
      const parts = relative.split("/").filter(Boolean);
      if (!recursive && parts.length !== 1) return;
      if (!parts.length) return;

      if (seen.has(path)) return;
      seen.add(path);
      pages.push({
        url,
        path,
        label: anchor.textContent.trim(),
        parts,
      });
    });

    return pages;
  };

  const extractPageHeadings = (html, levels) => {
    const doc = new DOMParser().parseFromString(html, "text/html");
    const title =
      doc.querySelector(".md-content h1, .md-typeset h1, h1")?.textContent?.trim() || "";
    const selectors = levels.flatMap((level) => [
      `.md-content h${level}[id]`,
      `.md-typeset h${level}[id]`,
      `h${level}[id]`,
    ]);
    const headings = [...doc.querySelectorAll(selectors.join(", "))]
      .map((heading) => ({
        level: Number(heading.tagName.slice(1)),
        id: heading.id,
        text: heading.textContent.trim(),
      }))
      .filter(
        (heading) =>
          levels.includes(heading.level) &&
          heading.id !== "__comments" &&
          heading.text !== "评论"
      );

    return { title, headings };
  };

  const renderHeadingList = (headings) => {
    if (!headings.length) return null;

    const root = document.createElement("ul");
    root.className = "directory-question-index__list";

    const baseLevel = Math.min(...headings.map((heading) => heading.level));
    const lists = [root];
    const parents = [];

    headings.forEach((item) => {
      const depth = Math.max(item.level - baseLevel, 0);

      lists.length = Math.min(lists.length, depth + 1);
      parents.length = Math.min(parents.length, depth);

      while (lists.length < depth + 1) {
        const parentItem = parents[parents.length - 1];
        if (!parentItem) break;

        let nestedList = parentItem.querySelector(":scope > .directory-question-index__list");
        if (!nestedList) {
          nestedList = document.createElement("ul");
          nestedList.className = "directory-question-index__list directory-question-index__list--nested";
          parentItem.appendChild(nestedList);
        }

        lists.push(nestedList);
      }

      const li = document.createElement("li");
      li.className = `directory-question-index__item directory-question-index__item--level-${item.level}`;

      const link = document.createElement("a");
      link.href = item.url;
      link.textContent = item.text;
      li.appendChild(link);

      (lists[depth] || root).appendChild(li);
      parents[depth] = li;
    });

    return root;
  };

  const renderPageSection = (section, headingTag = "h3") => {
    const block = document.createElement("section");
    block.className = "directory-question-index__section";

    const heading = document.createElement(headingTag);
    heading.className = "directory-question-index__heading";
    const headingLink = document.createElement("a");
    headingLink.href = section.url;
    headingLink.textContent = section.title || section.url;
    heading.appendChild(headingLink);
    block.appendChild(heading);

    if (section.headings.length) {
      const list = renderHeadingList(
        section.headings.map((item) => ({
          ...item,
          url: `${section.url}#${item.id}`,
        }))
      );
      if (list) {
        block.appendChild(list);
      }
    }

    return block;
  };

  const renderGroupedSections = (sections, groupBy) => {
    const fragment = document.createDocumentFragment();
    const groupMap = new Map();

    sections.forEach((section) => {
      const key = groupBy === "section" ? section.parts[0] : "";
      if (!groupMap.has(key)) {
        groupMap.set(key, []);
      }
      groupMap.get(key).push(section);
    });

    groupMap.forEach((items, key) => {
      if (!groupBy) {
        items.forEach((item) => fragment.appendChild(renderPageSection(item)));
        return;
      }

      const groupBlock = document.createElement("section");
      groupBlock.className = "directory-question-index__group";

      const deeperItems = items.filter((item) => item.parts.length > 1);
      const maybeIndex = items.find((item) => item.parts.length === 1);
      const groupTitle = maybeIndex?.title || maybeIndex?.label || prettifySegment(key);

      const groupHeading = document.createElement("h3");
      groupHeading.className = "directory-question-index__group-heading";
      if (maybeIndex) {
        const groupLink = document.createElement("a");
        groupLink.href = maybeIndex.url;
        groupLink.textContent = groupTitle;
        groupHeading.appendChild(groupLink);
      } else {
        groupHeading.textContent = groupTitle;
      }
      groupBlock.appendChild(groupHeading);

      const filesToRender = deeperItems.length ? deeperItems : items;
      filesToRender.forEach((item) => {
        if (maybeIndex && item.url === maybeIndex.url && deeperItems.length) return;
        groupBlock.appendChild(renderPageSection(item, "h4"));
      });

      fragment.appendChild(groupBlock);
    });

    return fragment;
  };

  const renderDirectoryIndex = async (container) => {
    const currentPath = normalizePath(window.location.pathname);
    const dirPath = currentPath;
    const levels = (container.dataset.level || "2")
      .split(",")
      .map((part) => Number(part.trim()))
      .filter(Boolean);
    const recursive = container.dataset.recursive === "true";
    const groupBy = container.dataset.groupBy || "";

    const pages = findSiblingPages(dirPath, currentPath, recursive);
    if (!pages.length) return;

    const sections = await Promise.all(
      pages.map(async (page) => {
        try {
          const response = await fetch(page.url.href);
          if (!response.ok) return null;
          const html = await response.text();
          return {
            url: normalizePath(page.url.pathname),
            label: page.label,
            parts: page.parts,
            ...extractPageHeadings(html, levels),
          };
        } catch {
          return null;
        }
      })
    );

    const validSections = sections.filter(Boolean);
    if (!validSections.length) return;

    container.replaceChildren(renderGroupedSections(validSections, groupBy));
  };

  const init = () => {
    document.querySelectorAll("[data-auto-dir-index]").forEach((container) => {
      renderDirectoryIndex(container);
    });
  };

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", init, { once: true });
  } else {
    init();
  }
})();
