# SIH2026 FastAPI Server

A modular, production-ready backend server built with [FastAPI](https://fastapi.tiangolo.com/), [Pydantic v2](https://docs.pydantic.dev/), and [Uvicorn](https://www.uvicorn.org/).

---

## 📁 Directory Structure

```
server/
├── app/
│   ├── __init__.py
│   ├── main.py                # FastAPI app initialization, CORS, static mounts
│   ├── core/
│   │   ├── __init__.py
│   │   └── config.py          # Environment configuration (Pydantic BaseSettings)
│   ├── api/
│   │   ├── __init__.py
│   │   └── v1/
│   │       ├── __init__.py
│   │       ├── api.py         # Router aggregation for API v1
│   │       └── endpoints/
│   │           ├── __init__.py
│   │           ├── health.py  # System health check endpoint
│   │           └── camera.py  # Camera frame upload & stream status endpoints
│   ├── schemas/
│   │   ├── __init__.py
│   │   ├── camera.py          # Camera & stream schemas
│   │   └── response.py        # Generic API response wrapper
│   └── services/              # Business logic & services
├── .env                       # Active environment variables
├── .env.example               # Example configuration template
├── .gitignore                 # Git ignore configuration
├── requirements.txt           # Python dependencies
└── README.md                  # Project documentation
```

---

## ⚙️ Quick Start Guide

### 1. Create Virtual Environment

From inside the `server/` directory:

```bash
cd server
python3 -m venv venv
source venv/bin/activate
```

### 2. Install Dependencies

```bash
pip install -r requirements.txt
```

### 3. Environment Setup

Check or edit `.env` file (copied from `.env.example`):

```ini
PROJECT_NAME="SIH2026 FastAPI Server"
VERSION="0.1.0"
API_V1_STR="/api/v1"
ENV="development"
HOST="0.0.0.0"
PORT=8000
BACKEND_CORS_ORIGINS=["*"]
UPLOAD_DIR="uploads"
```

### 4. Run Development Server

```bash
# Using uvicorn directly with hot-reload enabled:
uvicorn app.main:app --reload --host 0.0.0.0 --port 8000

# Or execute main.py directly:
python -m app.main
```

The server will be available at: `http://localhost:8000`

---

## 📖 Interactive Documentation

FastAPI automatically generates interactive OpenAPI documentation:

- **Swagger UI**: [http://localhost:8000/docs](http://localhost:8000/docs)
- **ReDoc**: [http://localhost:8000/redoc](http://localhost:8000/redoc)

---

## 🚀 Key API Endpoints

| Method | Endpoint | Description |
| :--- | :--- | :--- |
| `GET` | `/` | API Root landing & metadata |
| `GET` | `/health` | Quick health status |
| `GET` | `/api/v1/health` | Detailed health & system status |
| `POST` | `/api/v1/camera/upload` | Upload camera frame snapshot |
| `GET` | `/api/v1/camera/status` | Get stream status & frame stats |
| `GET` | `/api/v1/camera/latest` | Download latest uploaded camera frame |
| `GET` | `/uploads/{filename}` | Access uploaded files static server |

---

## 🧪 Testing Endpoints with curl

**Health Check**:
```bash
curl -X GET http://localhost:8000/api/v1/health
```

**Upload Image Snapshot**:
```bash
curl -X POST http://localhost:8000/api/v1/camera/upload \
  -F "file=@/path/to/test_image.jpg"
```

**Get Stream Status**:
```bash
curl -X GET http://localhost:8000/api/v1/camera/status
```
