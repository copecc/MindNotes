
// Generic 
// window.MathJax = {
//   tex: {
//     inlineMath: [["\\(", "\\)"]],
//     displayMath: [["\\[", "\\]"]],
//     processEscapes: true,
//     processEnvironments: true
//   },
//   options: {
//     ignoreHtmlClass: ".*|",
//     processHtmlClass: "arithmatex",
//   },
//   output: {
//     displayOverflow: 'linebreak',  // break long lines
//     linebreaks: {                  // options for when overflow is linebreak
//       inline: false,               // false for browser-based breaking of in-line equations
//       width: '100%',               // a fixed size or a percentage of the container width
//       lineleading: .2,             // the default lineleading in em units
//       LinebreakVisitor: null,      // The LinebreakVisitor to use
//     }
//   }
// };

// Script
window.MathJax = {
  options: {
    ignoreHtmlClass: 'tex2jax_ignore',
    processHtmlClass: 'tex2jax_process',
    renderActions: {
      find: [10, function (doc) {
        for (const node of document.querySelectorAll('script[type^="math/tex"]')) {
          const display = !!node.type.match(/; *mode=display/);
          const math = new doc.options.MathItem(node.textContent, doc.inputJax[0], display);
          const text = document.createTextNode('');
          const sibling = node.previousElementSibling;
          node.parentNode.replaceChild(text, node);
          math.start = { node: text, delim: '', n: 0 };
          math.end = { node: text, delim: '', n: 0 };
          doc.math.push(math);
          if (sibling && sibling.matches('.MathJax_Preview')) {
            sibling.parentNode.removeChild(sibling);
          }
        }
      }, '']
    }
  },
  output: {
    displayOverflow: 'linebreak',  // break long lines
    linebreaks: {                  // options for when overflow is linebreak
      inline: false,               // false for browser-based breaking of in-line equations
      width: '100%',               // a fixed size or a percentage of the container width
      lineleading: .2,             // the default lineleading in em units
      LinebreakVisitor: null,      // The LinebreakVisitor to use
    }
  }
};

// Re-render math on page change (for mkdocs-material with navigation.instant)
// document$.subscribe(() => {
//   MathJax.startup.output.clearCache()
//   MathJax.typesetClear()
//   MathJax.texReset()
//   MathJax.typesetPromise()
// })