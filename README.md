<div align="center">
  <img src="docs/favicon.svg" alt="Mind Notes Logo" width="96" />

  <h1>Mind Notes</h1>

  <p>个人技术博客与学习笔记仓库</p>

  <p>
    <a href="https://www.python.org/">
      <img src="https://img.shields.io/badge/python-3.12-blue" alt="Python 3.12" />
    </a>
    <a href="https://squidfunk.github.io/mkdocs-material/">
      <img src="https://img.shields.io/badge/MkDocs-Material-526CFE" alt="MkDocs Material" />
    </a>
    <a href="https://github.com/copecc/MindNotes/actions">
      <img src="https://img.shields.io/github/actions/workflow/status/copecc/MindNotes/ci.yml?branch=main" alt="Build Status" />
    </a>
    <a href="https://copecc.github.io/MindNotes/">
      <img src="https://img.shields.io/badge/GitHub%20Pages-deployed-brightgreen" alt="GitHub Pages" />
    </a>
  </p>

  <p>
    <a href="https://copecc.github.io/MindNotes/">
      <img src="https://img.shields.io/badge/%E5%9C%A8%E7%BA%BF%E9%98%85%E8%AF%BB-%E7%AB%8B%E5%8D%B3%E8%AE%BF%E9%97%AE-orange?style=for-the-badge" alt="在线阅读" />
    </a>
  </p>
</div>

---

## 📚 内容导航

这个仓库既是博客内容源，也是长期维护的知识库。内容主要分成四个入口：

| 入口 | 说明 |
| --- | --- |
| 📚 [学习笔记](docs/learning/) | 按主题整理技术笔记。 |
| 💻 [算法与代码](docs/code/) | 收录算法题、数据结构与代码实现说明，适合按问题和实现方式查阅。 |
| 🧠 [面试整理](docs/interview/) | 汇总后端、系统设计、项目与基础知识面试题，偏复盘与结构化总结。 |
| ✍️ [博客文章](docs/blog/) | 更偏文章化和连续阅读体验，适合从一个主题完整展开阅读。 |

如果是第一次进入这个仓库，比较推荐先从在线站点首页和 `docs/learning/` 开始。

## ✨ 仓库特点

这个仓库的重点不在于零散文章，而在于长期积累后的主题化整理。文档会尽量保持结构稳定，方便后续持续补充，也方便把同一类问题放回统一语境里理解。

内容组织上，代码示例会尽量和解释配套，适合边读边查；站点支持全文搜索、数学公式、Mermaid 图和代码复制，更适合在线浏览。仓库本身则保持了比较清晰的目录结构，方便本地维护和继续扩展。

## 🧰 技术栈

这个仓库主要使用下面这组工具链来构建和发布：

| 模块 | 技术 |
| --- | --- |
| 🧱 站点生成 | <img src="https://img.shields.io/badge/MkDocs-Documentation-2C3E50" alt="MkDocs" /> <img src="https://img.shields.io/badge/Material%20for%20MkDocs-Theme-526CFE" alt="Material for MkDocs" /> |
| ✍️ 内容格式 | <img src="https://img.shields.io/badge/Markdown-Content-000000" alt="Markdown" /> <img src="https://img.shields.io/badge/Mermaid-Diagram-FF3670" alt="Mermaid" /> <img src="https://img.shields.io/badge/MathJax-Formula-1F6FEB" alt="MathJax" /> |
| 🐍 本地环境 | <img src="https://img.shields.io/badge/Python-3.12-blue" alt="Python 3.12" /> |
| 🚀 持续集成与部署 | <img src="https://img.shields.io/badge/GitHub%20Actions-CI-2088FF" alt="GitHub Actions" /> <img src="https://img.shields.io/badge/GitHub%20Pages-Deploy-22C55E" alt="GitHub Pages" /> |

## 🚀 本地使用

如果你想在本地预览或继续维护这个仓库，可以直接执行：

```bash
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
mkdocs serve --livereload --dev-addr=127.0.0.1:8000
```

如果只想检查构建结果，可以执行：

```bash
mkdocs build
```

## 🗂️ 仓库结构

```text
docs/         文档正文
snippets/     供文档引用的代码片段
overrides/    MkDocs 主题覆盖与页面定制
mkdocs.yml    站点配置
main.py       MkDocs 宏与辅助逻辑
```

其中 `docs/` 是内容主目录，新增文档通常都应放在这里；`snippets/` 更适合存放会被复用的代码示例，而不是独立页面。

## 🛠️ 仓库维护方式

这个仓库会持续调整目录、补充笔记并重构旧文内容，所以提交记录里会出现较多“整理”“拆分”“重写”类变更。这些改动主要是为了让已有内容更适合长期维护，而不只是调整样式。
