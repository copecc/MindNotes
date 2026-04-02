---
hide:
  - navigation
  - toc
comments: false
---

# Mind Notes

{% set stats = homepage_stats() %}
{% set sections = homepage_sections() %}
<div class="home-shell">
  <section class="home-top">
    <div class="home-hero">
      <p class="home-kicker">Knowledge Base</p>
      <p class="home-tagline">个人博客 / 学习笔记 / 灵感记录</p>
      <p class="home-intro">
        收录算法题解、学习笔记、面试问题整理，以及零散但值得保留的技术随笔。
      </p>
    </div>

    <aside class="home-overview">
      <p class="home-overview__kicker">Site Overview</p>
      <div class="home-overview__panel">
        <div class="home-overview__summary">
          <div class="home-overview__summary-item">
            <span class="home-overview__summary-label">内容页</span>
            <strong class="home-overview__summary-value">{{ stats.document_count }}</strong>
          </div>
          <div class="home-overview__summary-item home-overview__summary-item--date">
            <span class="home-overview__summary-label">最近更新</span>
            <strong class="home-overview__summary-value">{{ stats.latest_update }}</strong>
          </div>
        </div>

        <div class="home-overview__meta">
          <div class="home-stat">
            <span class="home-stat__value">{{ stats.section_count }}</span>
            <span class="home-stat__label">主题</span>
          </div>
          <div class="home-stat">
            <span class="home-stat__value">{{ stats.code_count }}</span>
            <span class="home-stat__label">代码与算法</span>
          </div>
          <div class="home-stat">
            <span class="home-stat__value">{{ stats.recent_30d_count }}</span>
            <span class="home-stat__label">月更新</span>
          </div>
          <div class="home-stat">
            <span class="home-stat__value">{{ stats.recent_30d_added_count }}</span>
            <span class="home-stat__label">月新增</span>
          </div>
        </div>
      </div>
    </aside>
  </section>

  <section class="home-section">
    <div class="home-section__head">
      <p class="home-section__kicker">Explore</p>
      <h2 class="home-section__title">主题入口</h2>
    </div>

    <div class="home-strip">
      <button class="home-strip__control home-strip__control--prev" type="button" aria-label="上一个主题" data-scroll-target="home-sections-track" data-scroll-direction="-1">←</button>
      <div class="home-grid" id="home-sections-track" data-scroll-track>
        {% for section in sections %}
        <a class="home-card" href="{{ section.url | relative_url }}">
          <span class="home-card__eyebrow">{{ section.eyebrow }}</span>
          <strong class="home-card__title">{{ section.title }}</strong>
          <span class="home-card__body">{{ section.description }}</span>
        </a>
        {% endfor %}
      </div>
      <button class="home-strip__control home-strip__control--next" type="button" aria-label="下一个主题" data-scroll-target="home-sections-track" data-scroll-direction="1">→</button>
    </div>
  </section>

  <section class="home-section">
    <div class="home-section__head">
      <p class="home-section__kicker">Recent</p>
      <h2 class="home-section__title">最近更新</h2>
    </div>

    {% set recent_updates = homepage_recent_updates(6) %}
    <div class="home-updates">
      {% for item in recent_updates %}
      <a class="home-update" href="{{ item.url | relative_url }}">
        <span class="home-update__section">{{ item.section_label }}</span>
        <strong class="home-update__title">{{ item.title }}</strong>
        <span class="home-update__date">{{ item.updated_label }}</span>
        <span class="home-update__arrow">↗</span>
      </a>
      {% endfor %}
    </div>
  </section>
</div>
