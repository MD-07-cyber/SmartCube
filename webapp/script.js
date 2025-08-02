fetch('http://localhost:5000/mood-log')
  .then(res => res.json())
  .then(data => {
    const dates = Object.keys(data);
    const moods = Object.values(data);

    // Show today's mood
    const today = new Date().toISOString().slice(0, 10);
    document.getElementById("moodToday").textContent = `Today’s Mood: ${data[today] || 'No data yet'}`;

    // Chart
    new Chart(document.getElementById("moodChart"), {
      type: 'bar',
      data: {
        labels: dates,
        datasets: [{
          label: 'Daily Moods',
          data: moods.map(mood => {
            const moodScores = { happy: 3, neutral: 2, sad: 1, angry: 0 };
            return moodScores[mood] || 0;
          }),
          backgroundColor: moods.map(mood => {
            const colors = {
              happy: 'gold',
              sad: 'skyblue',
              angry: 'red',
              neutral: 'gray'
            };
            return colors[mood] || 'black';
          })
        }]
      },
      options: {
        scales: {
          y: {
            beginAtZero: true,
            ticks: {
              callback: function(value) {
                return ["😡", "😢", "😐", "😊"][value];
              }
            }
          }
        }
      }
    });
  });
