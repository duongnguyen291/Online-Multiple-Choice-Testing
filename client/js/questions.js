/**
 * Questions Management Module (Teacher Only)
 */

class QuestionsManager {
    constructor(wsClient) {
        this.wsClient = wsClient;
        this.questions = [];
        this.currentFilter = {
            search: '',
            subject: '',
            difficulty: ''
        };
        this.editingQuestion = null;
        
        this.initEventListeners();
    }

    initEventListeners() {
        // Add question button
        document.getElementById('btn-add-question')?.addEventListener('click', () => {
            this.showQuestionForm();
        });

        // Question form submit
        document.getElementById('question-form')?.addEventListener('submit', (e) => {
            e.preventDefault();
            this.handleQuestionFormSubmit();
        });

        // Modal close buttons
        document.querySelectorAll('.modal-close').forEach(btn => {
            btn.addEventListener('click', () => {
                this.closeModals();
            });
        });

        // Click outside modal to close
        document.querySelectorAll('.modal').forEach(modal => {
            modal.addEventListener('click', (e) => {
                if (e.target === modal) {
                    this.closeModals();
                }
            });
        });

        // Filter listeners
        document.getElementById('search-questions')?.addEventListener('input', (e) => {
            this.currentFilter.search = e.target.value.toLowerCase();
            this.filterQuestions();
        });

        document.getElementById('filter-subject')?.addEventListener('change', (e) => {
            this.currentFilter.subject = e.target.value;
            this.filterQuestions();
        });

        document.getElementById('filter-difficulty')?.addEventListener('change', (e) => {
            this.currentFilter.difficulty = e.target.value;
            this.filterQuestions();
        });

        // Register message handlers
        this.wsClient.on(S2C_QUESTIONS_LIST, (data) => {
            this.handleQuestionsList(data);
        });

        this.wsClient.on(S2C_QUESTION_CREATED, (data) => {
            this.handleQuestionCreated(data);
        });

        this.wsClient.on(S2C_QUESTION_UPDATED, (data) => {
            this.handleQuestionUpdated(data);
        });

        this.wsClient.on(S2C_QUESTION_DELETED, (data) => {
            this.handleQuestionDeleted(data);
        });
    }

    loadQuestions() {
        this.wsClient.send(C2S_LIST_QUESTIONS, {});
    }

    handleQuestionsList(data) {
        this.questions = data.questions || [];
        this.renderQuestions();
    }

    filterQuestions() {
        const filtered = this.questions.filter(q => {
            const matchSearch = !this.currentFilter.search ||
                q.question_text.toLowerCase().includes(this.currentFilter.search) ||
                q.option_a.toLowerCase().includes(this.currentFilter.search) ||
                q.option_b.toLowerCase().includes(this.currentFilter.search) ||
                q.option_c.toLowerCase().includes(this.currentFilter.search) ||
                q.option_d.toLowerCase().includes(this.currentFilter.search);
            
            const matchSubject = !this.currentFilter.subject ||
                q.subject === this.currentFilter.subject;
            
            const matchDifficulty = !this.currentFilter.difficulty ||
                q.difficulty === this.currentFilter.difficulty;
            
            return matchSearch && matchSubject && matchDifficulty;
        });

        this.renderQuestions(filtered);
    }

    renderQuestions(questionsToRender = null) {
        const container = document.getElementById('questions-list');
        if (!container) return;

        const questions = questionsToRender || this.questions;

        if (questions.length === 0) {
            container.innerHTML = `
                <div style="text-align: center; padding: 40px; color: var(--text-secondary);">
                    <p style="font-size: 18px;">📝 Chưa có câu hỏi nào</p>
                    <p>Nhấn nút "Thêm Câu Hỏi Mới" để bắt đầu</p>
                </div>
            `;
            return;
        }

        container.innerHTML = questions.map(q => `
            <div class="question-item" data-id="${q.question_id}">
                <div class="question-header">
                    <div class="question-meta">
                        <span class="badge badge-subject">${q.subject}</span>
                        <span class="badge badge-${q.difficulty.toLowerCase()}">${this.getDifficultyText(q.difficulty)}</span>
                    </div>
                    <div class="question-actions">
                        <button class="btn-icon btn-edit" onclick="questionsManager.editQuestion(${q.question_id})">
                            ✏️ Sửa
                        </button>
                        <button class="btn-icon btn-delete" onclick="questionsManager.deleteQuestion(${q.question_id})">
                            🗑️ Xóa
                        </button>
                    </div>
                </div>
                <div class="question-text">${q.question_text}</div>
                <div class="question-options">
                    <div class="option ${q.correct_answer === 'A' ? 'correct' : ''}">A. ${q.option_a}</div>
                    <div class="option ${q.correct_answer === 'B' ? 'correct' : ''}">B. ${q.option_b}</div>
                    <div class="option ${q.correct_answer === 'C' ? 'correct' : ''}">C. ${q.option_c}</div>
                    <div class="option ${q.correct_answer === 'D' ? 'correct' : ''}">D. ${q.option_d}</div>
                </div>
            </div>
        `).join('');
    }

    getDifficultyText(difficulty) {
        const map = {
            'EASY': 'Dễ',
            'MEDIUM': 'Trung bình',
            'HARD': 'Khó'
        };
        return map[difficulty] || difficulty;
    }

    showQuestionForm(question = null) {
        const modal = document.getElementById('modal-question-form');
        const form = document.getElementById('question-form');
        const title = document.getElementById('question-form-title');

        this.editingQuestion = question;

        if (question) {
            // Edit mode
            title.textContent = 'Chỉnh Sửa Câu Hỏi';
            document.getElementById('question-id').value = question.question_id;
            document.getElementById('question-subject').value = question.subject;
            document.getElementById('question-difficulty').value = question.difficulty;
            document.getElementById('question-text').value = question.question_text;
            document.getElementById('option-a').value = question.option_a;
            document.getElementById('option-b').value = question.option_b;
            document.getElementById('option-c').value = question.option_c;
            document.getElementById('option-d').value = question.option_d;
            document.getElementById('correct-answer').value = question.correct_answer;
        } else {
            // Create mode
            title.textContent = 'Thêm Câu Hỏi Mới';
            form.reset();
            document.getElementById('question-id').value = '';
        }

        modal.classList.add('active');
    }

    closeModals() {
        document.querySelectorAll('.modal').forEach(modal => {
            modal.classList.remove('active');
        });
        this.editingQuestion = null;
    }

    handleQuestionFormSubmit() {
        const questionId = document.getElementById('question-id').value;
        const data = {
            subject: document.getElementById('question-subject').value,
            difficulty: document.getElementById('question-difficulty').value,
            question_text: document.getElementById('question-text').value,
            option_a: document.getElementById('option-a').value,
            option_b: document.getElementById('option-b').value,
            option_c: document.getElementById('option-c').value,
            option_d: document.getElementById('option-d').value,
            correct_answer: document.getElementById('correct-answer').value
        };

        if (questionId) {
            // Update existing question
            data.question_id = parseInt(questionId);
            this.wsClient.send(C2S_UPDATE_QUESTION, data);
        } else {
            // Create new question
            this.wsClient.send(C2S_CREATE_QUESTION, data);
        }

        this.closeModals();
    }

    editQuestion(questionId) {
        const question = this.questions.find(q => q.question_id === questionId);
        if (question) {
            this.showQuestionForm(question);
        }
    }

    deleteQuestion(questionId) {
        if (confirm('Bạn có chắc chắn muốn xóa câu hỏi này?')) {
            this.wsClient.send(C2S_DELETE_QUESTION, { question_id: questionId });
        }
    }

    handleQuestionCreated(data) {
        showNotification('Đã tạo câu hỏi mới thành công!', 'success');
        this.loadQuestions();
    }

    handleQuestionUpdated(data) {
        showNotification('Đã cập nhật câu hỏi thành công!', 'success');
        this.loadQuestions();
    }

    handleQuestionDeleted(data) {
        showNotification('Đã xóa câu hỏi thành công!', 'success');
        this.loadQuestions();
    }
}

// Global instance (will be initialized in main.js)
let questionsManager = null;
